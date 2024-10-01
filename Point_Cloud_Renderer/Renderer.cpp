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

#include "Renderer/Structures/FrameData.h"
#include "Renderer/Structures/InstanceData.h"
#include "Renderer/Structures/CameraData.h"
#include "Math/Utility.hpp"

namespace PCR
{
    Renderer::Renderer( MTL::Device* pDevice )
    :   _pDevice( pDevice->retain() )
    ,   _frame( 0 )
    ,   _angle( 0.0f )
    {
        _pCommandQueue = _pDevice->newCommandQueue();
        buildShaders();
        buildDepthStencilStates();
        buildBuffers();
        
        _semaphore = dispatch_semaphore_create( MAX_FRAMES_IN_FLIGHT );
    }

    Renderer::~Renderer()
    {
        _pShaderLibrary->release();
        _pDepthStencilState->release();
        _pVertexDataBuffer->release();
        _pIndexBuffer->release();
        for ( int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
        {
            _pInstanceDataBuffers[ i ]->release();
        }
        _pRenderPipelineStateObject->release();
        _pCommandQueue->release();
        _pDevice->release();
    }

    void Renderer::draw( MTK::View* pView )
    {
        NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();
        
        _frame = (_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        MTL::Buffer* pCurrentInstanceDataBuffer = _pInstanceDataBuffers[ _frame ];

        MTL::CommandBuffer* pCmd = _pCommandQueue->commandBuffer();
        dispatch_semaphore_wait( _semaphore, DISPATCH_TIME_FOREVER );
        pCmd->addCompletedHandler( ^void( MTL::CommandBuffer* pCmd ){
            dispatch_semaphore_signal( this->_semaphore );
        });
        
        _angle += 0.01f;
        
        const float scl = 0.1f;
        InstanceData* pInstanceData = reinterpret_cast< InstanceData* >( pCurrentInstanceDataBuffer->contents() );
        
        simd::float3 cameraPosition{ 0.0f, 0.0f, -5.0f };
        
        simd::float4x4 rt = Math::makeTranslate( cameraPosition );
        simd::float4x4 rr = Math::makeYRotate( -_angle );
        simd::float4x4 rtInv = Math::makeTranslate( { -cameraPosition.x, -cameraPosition.y, -cameraPosition.z } );
        simd::float4x4 fullObjectRot = rt * rr * rtInv;
        
        // Instance Data
        for ( size_t i = 0; i < MAX_NUM_INSTANCES; ++i )
        {
            float iDivNumInstances = i / static_cast<float>( MAX_NUM_INSTANCES );
            float xoff = ( iDivNumInstances * 2.0f - 1.0f ) + ( 1.0f / MAX_NUM_INSTANCES );
            float yoff = sin( ( iDivNumInstances + _angle ) * 2.0f * M_PI );
            
            simd::float4x4 translate = Math::makeTranslate( cameraPosition + simd::float3{ xoff, yoff, 0.0f } );
            simd::float4x4 zRotation = Math::makeZRotate( _angle );
            simd::float4x4 yRotation = Math::makeZRotate( _angle );
            simd::float4x4 scale = Math::makeScale( simd::float3{ scl, scl, scl } );
            
            // Instance Transform
            pInstanceData[ i ].transform = fullObjectRot * translate * yRotation * zRotation * scale;

            float r = iDivNumInstances;
            float g = 1.0f - r;
            float b = sinf( M_PI * 2.0f * iDivNumInstances );
            
            // Instance Color
            pInstanceData[ i ].color = { r, g, b, 1.0f };
        }
        pCurrentInstanceDataBuffer->didModifyRange( NS::Range::Make( 0, pCurrentInstanceDataBuffer->length() ) );
        
        // Update Camera State
        
        MTL::Buffer* pCurrentCameraBuffer = _pCameraDataBuffers[ _frame ];
        auto* pCameraData = reinterpret_cast< CameraData* >( pCurrentCameraBuffer->contents() );
        pCameraData->perspectiveTransform = Math::makePerspective( 45.0f * M_PI / 180.0f, 1.0f, 0.03f, 500.0f );
        pCameraData->worldTransform = Math::makeIdentity();
        pCurrentCameraBuffer->didModifyRange( NS::Range::Make( 0, pCurrentCameraBuffer->length() ) );
        
        // Begin Render Pass
        
        MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
        MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder( pRpd );
        
        pEnc->setRenderPipelineState( _pRenderPipelineStateObject );
        pEnc->setVertexBuffer( _pVertexDataBuffer, 0, 0 );
        pEnc->setVertexBuffer( pCurrentInstanceDataBuffer, 0, 1 );
        pEnc->setVertexBuffer( pCurrentCameraBuffer, 0, 2 );
        
        pEnc->setCullMode( MTL::CullModeBack );
        pEnc->setFrontFacingWinding( MTL::Winding::WindingCounterClockwise );
        
        // Draw-call
        pEnc->drawIndexedPrimitives( MTL::PrimitiveType::PrimitiveTypeTriangle,
                                     /* indexCount */ 6 * 6,
                                     MTL::IndexType::IndexTypeUInt16,
                                     _pIndexBuffer,
                                     /* indexBufferOffset */ 0,
                                     MAX_NUM_INSTANCES);
        
        pEnc->endEncoding();
        pCmd->presentDrawable( pView->currentDrawable() );
        pCmd->commit();

        pPool->release();
    }
    
    void Renderer::buildShaders()
    {
        NS::String* shaderSource = CreateUTF8String( R"(
            #include <metal_stdlib>
            using namespace metal;

            struct v2f
            {
                float4 position [[position]];
                half3 color;
            };
        
            struct VertexData
            {
                float3 position;
            };
        
            struct InstanceData
            {
                float4x4 transform;
                float4 color;
            };
        
            struct CameraData
            {
                float4x4 perspectiveTransform;
                float4x4 worldTransform;
            };

            v2f vertex vertexMain( uint vertexId   [[vertex_id]],
                                   uint instanceID [[instance_id]],
                                   device const VertexData*   vertexData   [[buffer(0)]],
                                   device const InstanceData* instanceData [[buffer(1)]],
                                   device const CameraData*   cameraData   [[buffer(2)]] )
            {
                v2f o;
        
                float4 pos = float4( vertexData[ vertexId ].position, 1.0 );
                pos = instanceData[ instanceID ].transform * pos;
                pos = cameraData->perspectiveTransform * cameraData->worldTransform * pos;
        
                o.position = pos;
                o.color = half3 ( instanceData[ instanceID ].color.rgb );
        
                return o;
            }

            half4 fragment fragmentMain( v2f in [[stage_in]] )
            {
                return half4( in.color, 1.0 );
            }
        )" );
        
        NS::Error* pError = nullptr;
        MTL::Library* pLibrary = _pDevice->newLibrary( shaderSource, nullptr, &pError );
        if ( !pLibrary )
        {
            __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
            assert( false );
        }
        
        MTL::Function* pVertexMainFn = pLibrary->newFunction( CreateUTF8String("vertexMain") );
        MTL::Function* pFragmentMainFn = pLibrary->newFunction( CreateUTF8String("fragmentMain") );
        
        MTL::RenderPipelineDescriptor* pRenderPipelineDesc = MTL::RenderPipelineDescriptor::alloc()->init();
        pRenderPipelineDesc->setVertexFunction( pVertexMainFn );
        pRenderPipelineDesc->setFragmentFunction( pFragmentMainFn );
        pRenderPipelineDesc->colorAttachments()->object( 0 )->setPixelFormat( MTL::PixelFormatBGRA8Unorm_sRGB );
        
        _pRenderPipelineStateObject = _pDevice->newRenderPipelineState( pRenderPipelineDesc, &pError );
        if ( !_pRenderPipelineStateObject )
        {
            __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
            assert( false );
        }
        
        pVertexMainFn->release();
        pFragmentMainFn->release();
        pRenderPipelineDesc->release();
        _pShaderLibrary = pLibrary;
    }
    
    void Renderer::buildBuffers()
    {
        constexpr float s = 0.5f;
        
        constexpr simd::float3 vertices[] = {
            { -s, -s, +s },
            { +s, -s, +s },
            { +s, +s, +s },
            { -s, +s, +s },

            { -s, -s, -s },
            { -s, +s, -s },
            { +s, +s, -s },
            { +s, -s, -s }
        };
        
        constexpr uint16_t indices[] = {
            0, 1, 2, /* front */
            2, 3, 0,

            1, 7, 6, /* right */
            6, 2, 1,

            7, 4, 5, /* back */
            5, 6, 7,

            4, 0, 3, /* left */
            3, 5, 4,

            3, 2, 6, /* top */
            6, 5, 3,

            4, 7, 1, /* bottom */
            1, 0, 4
        };
        
        constexpr size_t vertexDataSize = sizeof( vertices );
        constexpr size_t indexDataSize = sizeof( indices );
        
        _pVertexDataBuffer = _pDevice->newBuffer( vertexDataSize, MTL::ResourceStorageModeManaged );
        _pIndexBuffer = _pDevice->newBuffer( indexDataSize, MTL::ResourceStorageModeManaged );
        
        memcpy( _pVertexDataBuffer->contents(), vertices, vertexDataSize );
        memcpy( _pIndexBuffer->contents(), indices, indexDataSize );
        
        _pVertexDataBuffer->didModifyRange( NS::Range::Make( 0, _pVertexDataBuffer->length() ) );
        _pIndexBuffer->didModifyRange( NS::Range::Make( 0, _pIndexBuffer->length() ) );
        
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
    }
    
    void Renderer::buildDepthStencilStates()
    {
        MTL::DepthStencilDescriptor* pDepthStencilDescriptor = MTL::DepthStencilDescriptor::alloc()->init();
        pDepthStencilDescriptor->setDepthCompareFunction( MTL::CompareFunction::CompareFunctionLess );
        pDepthStencilDescriptor->setDepthWriteEnabled( true );
        
        _pDepthStencilState = _pDevice->newDepthStencilState( pDepthStencilDescriptor );
        
        pDepthStencilDescriptor->release();
    }
}
