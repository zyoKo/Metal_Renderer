//
//  Renderer.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#include "Renderer.hpp"

#include <cassert>

#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>
#include <simd/simd.h>

#include "Renderer/Structures/FrameData.hpp"
#include "Renderer/Structures/InstanceData.hpp"
#include "Renderer/Structures/CameraData.hpp"
#include "Math/Utility.hpp"
#include "Renderer/Mesh/Types/VertexData.h"
#include "Renderer/Mesh/Mesh.hpp"
#include "Renderer/Mesh/MeshFactory.hpp"
#include "Renderer/Mesh/SubMesh.hpp"
#include "Renderer/Structures/ShaderTypes.hpp"
#include "Window/Input/Input.hpp"

namespace PCR
{
    Renderer::Renderer( MTL::Device* pDevice )
    :   _pDevice{ pDevice->retain() }
    ,   _frame{ 0 }
    ,   _angle{ 0.0f }
    ,   _animationIndex{ 0 }
    ,   _activeMeshIndex{ 0 }
    {
        _pCommandQueue = _pDevice->newCommandQueue();
        buildShaders();
        buildDepthStencilStates();
        buildComputePipeline();
        buildTextures();
        buildBuffers();
        buildMeshes();
        
        _semaphore = dispatch_semaphore_create( MAX_FRAMES_IN_FLIGHT );
    }

    Renderer::~Renderer()
    {
        _pTexture->release();
        _pShaderLibrary->release();
        _pDepthStencilState->release();
        _pTextureAnimationBuffer->release();
        for ( int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
        {
            _pInstanceDataBuffers[ i ]->release();
            _pCameraDataBuffers[ i ]->release();
        }
        _pComputePipelineStateObject->release();
        _pRenderPipelineStateObject->release();
        _pCommandQueue->release();
        _pDevice->release();
    }

    void Renderer::draw( MTK::View* pView )
    {
        auto pAutoReleasePool = NS::TransferPtr< NS::AutoreleasePool >( NS::AutoreleasePool::alloc()->init() );
        
        handleInput();
        
        _frame = (_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        MTL::Buffer* pCurrentInstanceDataBuffer = _pInstanceDataBuffers[ _frame ];

        MTL::CommandBuffer* pCommandBuffer = _pCommandQueue->commandBuffer();
        dispatch_semaphore_wait( _semaphore, DISPATCH_TIME_FOREVER );
        pCommandBuffer->addCompletedHandler( ^void( MTL::CommandBuffer* pCmd ){
            dispatch_semaphore_signal( this->_semaphore );
        });
        
        _angle += 0.01f;
        
        constexpr float scl = 0.2f;
        constexpr float doubleScl = scl * 2.0f;
        InstanceData* pInstanceData = reinterpret_cast< InstanceData* >( pCurrentInstanceDataBuffer->contents() );
        
        simd::float3 cameraPosition{ 0.0f, 0.0f, -10.0f };
        
        simd::float4x4 rt = Math::makeTranslate( cameraPosition );
        simd::float4x4 rr1 = Math::makeYRotate( -_angle );
        simd::float4x4 rr0 = Math::makeYRotate( _angle * 0.5f );
        simd::float4x4 rtInv = Math::makeTranslate( { -cameraPosition.x, -cameraPosition.y, -cameraPosition.z } );
        simd::float4x4 fullObjectRot = rt * rr1 * rr0 * rtInv;
        
        size_t ix = 0;
        size_t iy = 0;
        size_t iz = 0;
        
        // Instance Data
        for ( size_t i = 0; i < MAX_NUM_INSTANCES; ++i )
        {
            if ( ix == INSTANCE_ROWS )
            {
                ix = 0;
                iy += 1;
            }
            
            if ( iy == INSTANCE_COLUMNS )
            {
                iy = 0;
                iz += 1;
            }
            
            auto fx = static_cast< float >( ix );
            auto fy = static_cast< float >( iy );
            auto fz = static_cast< float >( iz );
            
            constexpr float HALF_ROWS = static_cast< float >( INSTANCE_ROWS ) / 2.0f;
            constexpr float HALF_COLUMNS = static_cast< float >( INSTANCE_COLUMNS ) / 2.0f;
            constexpr float HALF_DEPTH = static_cast< float >( INSTANCE_DEPTH ) / 2.0f;
            
            // Instance Object Transform
            simd::float4x4 zRotation = Math::makeZRotate( _angle * sinf( fx ) );
            simd::float4x4 yRotation = Math::makeZRotate( _angle * cosf( fy ) );
            simd::float4x4 scale = Math::makeScale( simd::float3{ scl, scl, scl } );
            
            float x = ( fx - HALF_ROWS ) * doubleScl + scl;
            float y = ( fy - HALF_COLUMNS ) * doubleScl + scl;
            float z = ( fz - HALF_DEPTH ) * doubleScl;
            simd::float4x4 translate = Math::makeTranslate( cameraPosition + simd::float3{ x, y, z } );
            
            // Instance Transform
            pInstanceData[ i ].transform = fullObjectRot * translate * yRotation * zRotation * scale;
            pInstanceData[ i ].normalTransform = Math::discardTranslation( pInstanceData[ i ].transform );

            // Instance Color
            float iDivNumInstances = i / static_cast<float>( MAX_NUM_INSTANCES );
            float r = iDivNumInstances;
            float g = 1.0f - r;
            float b = sinf( M_PI * 2.0f * iDivNumInstances );
            pInstanceData[ i ].color = { r, g, b, 1.0f };
            
            ix += 1;
        }
        pCurrentInstanceDataBuffer->didModifyRange( NS::Range::Make( 0, pCurrentInstanceDataBuffer->length() ) );
        
        // Update Camera State
        
        MTL::Buffer* pCurrentCameraBuffer = _pCameraDataBuffers[ _frame ];
        auto* pCameraData = reinterpret_cast< CameraData* >( pCurrentCameraBuffer->contents() );
        pCameraData->perspectiveTransform = Math::makePerspective( 45.0f * M_PI / 180.0f, 1.0f, 0.03f, 500.0f );
        pCameraData->worldTransform = Math::makeIdentity();
        pCameraData->worldNormalTransform = Math::discardTranslation( pCameraData->worldTransform );
        pCurrentCameraBuffer->didModifyRange( NS::Range::Make( 0, pCurrentCameraBuffer->length() ) );
        
        // Update Texture
        
        generateMandelbrotTexture( pCommandBuffer );
        
        // Begin Render Pass
        
        MTL::RenderPassDescriptor* pRenderPassDescriptor = pView->currentRenderPassDescriptor();
        MTL::RenderCommandEncoder* pRenderCommandEncoder = pCommandBuffer->renderCommandEncoder( pRenderPassDescriptor );
        
        pRenderCommandEncoder->setRenderPipelineState( _pRenderPipelineStateObject );
        pRenderCommandEncoder->setDepthStencilState( _pDepthStencilState );
        
        /* MTL::Buffer*, offset, index */
        pRenderCommandEncoder->setVertexBuffer( pCurrentInstanceDataBuffer, 0, BufferIndexInstanceData );
        pRenderCommandEncoder->setVertexBuffer( pCurrentCameraBuffer, 0, BufferIndexCameraData );
        
        /* MTL::Texture*, index */
        pRenderCommandEncoder->setFragmentTexture( _pTexture, 0 );
        
        pRenderCommandEncoder->setCullMode( MTL::CullMode::CullModeBack );
        pRenderCommandEncoder->setFrontFacingWinding( MTL::Winding::WindingCounterClockwise );
        
        // Draw-call
        drawMesh( pRenderCommandEncoder, *_meshes[ _activeMeshIndex ].pMesh, MAX_NUM_INSTANCES );
        
        pRenderCommandEncoder->endEncoding();
        pCommandBuffer->presentDrawable( pView->currentDrawable() );
        pCommandBuffer->commit();
    }
    
    void Renderer::buildShaders()
    {
        auto pLibrary = NS::TransferPtr( _pDevice->newDefaultLibrary() );
        if ( !pLibrary )
        {
            assert( false );
        }
        
        auto pVertexMainFn = NS::TransferPtr( pLibrary->newFunction( CreateUTF8String( "vertexMain" ) ) );
        auto pFragmentMainFn = NS::TransferPtr( pLibrary->newFunction( CreateUTF8String( "fragmentMain" ) ) );
        
        auto pRenderPipelineDesc = NS::TransferPtr( MTL::RenderPipelineDescriptor::alloc()->init() );
        pRenderPipelineDesc->setVertexFunction( pVertexMainFn.get() );
        pRenderPipelineDesc->setFragmentFunction( pFragmentMainFn.get() );
        pRenderPipelineDesc->setVertexDescriptor( MeshFactory::makeVertexDescriptor().get() );
        pRenderPipelineDesc->colorAttachments()->object( 0 )->setPixelFormat( MTL::PixelFormatBGRA8Unorm_sRGB );
        pRenderPipelineDesc->setDepthAttachmentPixelFormat( MTL::PixelFormat::PixelFormatDepth16Unorm );
        
        NS::Error* pError = nullptr;
        _pRenderPipelineStateObject = _pDevice->newRenderPipelineState( pRenderPipelineDesc.get(), &pError );
        if ( !_pRenderPipelineStateObject )
        {
            __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
            assert( false );
        }
        
        _pShaderLibrary = pLibrary->retain();
    }
    
    void Renderer::buildBuffers()
    {
        constexpr size_t instanceDataSize = MAX_FRAMES_IN_FLIGHT * MAX_NUM_INSTANCES * sizeof( InstanceData );
        for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
        {
            _pInstanceDataBuffers[ i ] = _pDevice->newBuffer( instanceDataSize, MTL::ResourceStorageModeManaged );
        }
        
        constexpr size_t cameraDataSize = MAX_FRAMES_IN_FLIGHT * sizeof( CameraData );
        for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
        {
            _pCameraDataBuffers[ i ] = _pDevice->newBuffer( cameraDataSize, MTL::ResourceStorageModeManaged );
        }
        
        _pTextureAnimationBuffer = _pDevice->newBuffer( sizeof( uint ), MTL::ResourceStorageModeManaged );
    }
    
    void Renderer::buildMeshes()
    {
        // Position in this list = number key (1 = first).
        _meshes.push_back( { "Cube",            std::make_unique< Mesh >( MeshFactory::makeCube( _pDevice ) ) } );
        _meshes.push_back( { "Sphere",          std::make_unique< Mesh >( MeshFactory::makeSphere( _pDevice ) ) } );
        _meshes.push_back( { "Low-poly Sphere", std::make_unique< Mesh >( MeshFactory::makeSphere( _pDevice, 0.5f, 8, 4 ) ) } );
        _meshes.push_back( { "Pyramid",         std::make_unique< Mesh >( MeshFactory::makePyramid( _pDevice ) ) } );
        
        _activeMeshIndex = 0;
        __builtin_printf( "[Renderer] Mesh: %s (press 1-%zu to switch)\n", _meshes[ 0 ].name.c_str(), _meshes.size() );
    }
    
    void Renderer::handleInput()
    {
        const int requested = Input::numberKeyPressed();
        
        if ( requested < 0 || static_cast< size_t >( requested ) >= _meshes.size() )
        {
            return;
        }
        
        if ( static_cast< size_t >( requested ) != _activeMeshIndex )
        {
            _activeMeshIndex = static_cast< size_t >( requested );
            __builtin_printf( "[Renderer] Mesh: %s\n", _meshes[ _activeMeshIndex ].name.c_str() );
        }
    }
    
    void Renderer::drawMesh( MTL::RenderCommandEncoder* pEncoder,
                             const Mesh& mesh,
                             NS::UInteger instanceCount )
    {
        assert( pEncoder );
        
        for ( const MeshBuffer& vertexBuffer : mesh.GetMeshBuffers() )
        {
            pEncoder->setVertexBuffer( vertexBuffer.getBuffer(),
                                       vertexBuffer.getOffset(),
                                       vertexBuffer.getArgumentIndex() );
        }
        
        for ( const SubMesh& subMesh : mesh.GetSubMeshes() )
        {
            const MeshBuffer& indexBuffer = subMesh.getMeshBuffer();
            
            pEncoder->drawIndexedPrimitives( subMesh.getPrimitiveType(),
                                             subMesh.getIndexCount(),
                                             subMesh.getIndexType(),
                                             indexBuffer.getBuffer(),
                                             indexBuffer.getOffset(),
                                             instanceCount );
        }
    }
    
    void Renderer::buildDepthStencilStates()
    {
        auto pDepthStencilDescriptor = NS::TransferPtr( MTL::DepthStencilDescriptor::alloc()->init() );
        pDepthStencilDescriptor->setDepthCompareFunction( MTL::CompareFunction::CompareFunctionLess );
        pDepthStencilDescriptor->setDepthWriteEnabled( true );
        _pDepthStencilState = _pDevice->newDepthStencilState( pDepthStencilDescriptor.get() );
    }
    
    void Renderer::buildTextures()
    {
        auto pTextureDesc = NS::TransferPtr< MTL::TextureDescriptor >( MTL::TextureDescriptor::alloc()->init() );
        pTextureDesc->setWidth( DEFAULT_TEXTURE_WIDTH );
        pTextureDesc->setHeight( DEFAULT_TEXTURE_HEIGHT );
        pTextureDesc->setPixelFormat( MTL::PixelFormat::PixelFormatRGBA8Unorm );
        pTextureDesc->setTextureType( MTL::TextureType2D );
        pTextureDesc->setStorageMode( MTL::StorageModeManaged );
        pTextureDesc->setUsage( MTL::ResourceUsageSample | MTL::ResourceUsageRead | MTL::ResourceUsageWrite );

        _pTexture = _pDevice->newTexture( pTextureDesc.get() );
    }
    
    void Renderer::buildComputePipeline()
    {
        auto pComputeLibrary = NS::TransferPtr( _pDevice->newDefaultLibrary() );
        if ( !pComputeLibrary )
        {
            assert( false );
        }
        
        auto pMandelbrotFn = NS::TransferPtr< MTL::Function >( pComputeLibrary->newFunction( CreateUTF8String( "mandelbrot_set" ) ) );
        
        NS::Error* pError = nullptr;
        MTL::ComputePipelineState* pComputePipelineStateObject = _pDevice->newComputePipelineState( pMandelbrotFn.get(), &pError );
        if ( !pComputePipelineStateObject )
        {
            __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
            assert( false );
        }
        _pComputePipelineStateObject = pComputePipelineStateObject;
    }
    
    void Renderer::generateMandelbrotTexture( MTL::CommandBuffer* pCommandBuffer )
    {
        assert( pCommandBuffer );
        
        uint* ptr = reinterpret_cast< uint* >( _pTextureAnimationBuffer->contents() );
        *ptr = ( _animationIndex++ );
        if ( _animationIndex >= 5000 )
        {
            _animationIndex = 0;
        }
        _pTextureAnimationBuffer->didModifyRange( NS::Range( 0, sizeof( uint ) ) );
        
        MTL::ComputeCommandEncoder* pComputeEncoder = pCommandBuffer->computeCommandEncoder();
        
        pComputeEncoder->setComputePipelineState( _pComputePipelineStateObject );
        pComputeEncoder->setTexture( _pTexture, 0 );
        pComputeEncoder->setBuffer( _pTextureAnimationBuffer, /* offset */ 0 , /* index */ 0);
        
        MTL::Size gridSize = MTL::Size( DEFAULT_TEXTURE_WIDTH, DEFAULT_TEXTURE_HEIGHT, 1 );
        
        NS::UInteger threadGroupX = _pComputePipelineStateObject->maxTotalThreadsPerThreadgroup();
        MTL::Size threadGroupSize = MTL::Size( threadGroupX, 1, 1 );
        
        pComputeEncoder->dispatchThreads( gridSize, threadGroupSize );
        
        pComputeEncoder->endEncoding();
    }
}
