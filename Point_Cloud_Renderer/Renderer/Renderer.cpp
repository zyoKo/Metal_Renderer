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
#include "Renderer/Mesh/Types/VertexData.h"

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
        buildTextures();
        buildBuffers();
        
        _semaphore = dispatch_semaphore_create( MAX_FRAMES_IN_FLIGHT );
    }

    Renderer::~Renderer()
    {
        _pTexture->release();
        _pShaderLibrary->release();
        _pDepthStencilState->release();
        _pVertexDataBuffer->release();
        _pIndexBuffer->release();
        for ( int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
        {
            _pInstanceDataBuffers[ i ]->release();
            _pCameraDataBuffers[ i ]->release();
        }
        _pRenderPipelineStateObject->release();
        _pCommandQueue->release();
        _pDevice->release();
    }

    void Renderer::draw( MTK::View* pView )
    {
        auto pAutoReleasePool = NS::TransferPtr< NS::AutoreleasePool >( NS::AutoreleasePool::alloc()->init() );
        
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
        
        // Begin Render Pass
        
        MTL::RenderPassDescriptor* pRenderPassDescriptor = pView->currentRenderPassDescriptor();
        MTL::RenderCommandEncoder* pRenderCommandEncoder = pCommandBuffer->renderCommandEncoder( pRenderPassDescriptor );
        
        pRenderCommandEncoder->setRenderPipelineState( _pRenderPipelineStateObject );
        pRenderCommandEncoder->setDepthStencilState( _pDepthStencilState );
        
        /* MTL::Buffer*, offset, index */
        pRenderCommandEncoder->setVertexBuffer( _pVertexDataBuffer, 0, 0 );
        pRenderCommandEncoder->setVertexBuffer( pCurrentInstanceDataBuffer, 0, 1 );
        pRenderCommandEncoder->setVertexBuffer( pCurrentCameraBuffer, 0, 2 );
        
        /* MTL::Texture*, index */
        pRenderCommandEncoder->setFragmentTexture( _pTexture, 0 );
        
        pRenderCommandEncoder->setCullMode( MTL::CullMode::CullModeBack );
        pRenderCommandEncoder->setFrontFacingWinding( MTL::Winding::WindingCounterClockwise );
        
        // Draw-call
        pRenderCommandEncoder->drawIndexedPrimitives( MTL::PrimitiveType::PrimitiveTypeTriangle,
                                                     /* indexCount */ 6 * 6,
                                                     MTL::IndexType::IndexTypeUInt16,
                                                     _pIndexBuffer,
                                                     /* indexBufferOffset */ 0,
                                                     MAX_NUM_INSTANCES);
        
        pRenderCommandEncoder->endEncoding();
        pCommandBuffer->presentDrawable( pView->currentDrawable() );
        pCommandBuffer->commit();
    }
    
    void Renderer::buildShaders()
    {
        NS::String* shaderSource = CreateUTF8String( R"(
            #include <metal_stdlib>
            using namespace metal;

            struct v2f
            {
                float4 position [[ position ]];
                float3 normal;
                half3 color;
                float2 texCoord;
            };
        
            struct VertexData
            {
                float3 position;
                float3 normal;
                float2 texCoord;
            };
        
            struct InstanceData
            {
                float4x4 transform;
                float3x3 normalTransform;
                float4 color;
            };
        
            struct CameraData
            {
                float4x4 perspectiveTransform;
                float4x4 worldTransform;
                float3x3 worldNormalTransform;
            };

            v2f vertex vertexMain( uint vertexId   [[ vertex_id ]],
                                   uint instanceID [[ instance_id ]],
                                   device const VertexData*   vertexData   [[ buffer( 0 ) ]],
                                   device const InstanceData* instanceData [[ buffer( 1 ) ]],
                                   device const CameraData*   cameraData   [[ buffer( 2 ) ]] )
            {
                v2f o;
        
                const device VertexData& vd = vertexData[ vertexId ];
        
                float4 pos = float4( vd.position, 1.0 );
                pos = instanceData[ instanceID ].transform * pos;
                pos = cameraData->perspectiveTransform * cameraData->worldTransform * pos;
                o.position = pos;
        
                float3 normal = instanceData[ instanceID ].normalTransform * vd.normal;
                normal = cameraData->worldNormalTransform * normal;
                o.normal = normal;
        
                o.texCoord = vd.texCoord;
        
                o.color = half3 ( instanceData[ instanceID ].color.rgb );
        
                return o;
            }

            half4 fragment fragmentMain( v2f in [[stage_in]], texture2d< half, access::sample > tex [[ texture( 0 ) ]] )
            {
                constexpr sampler s( address::repeat, filter::linear );
                half3 texel = tex.sample( s, in.texCoord ).rgb;
        
                // assuming light coming from ( front-top-right )
                float3 L = normalize( float3( 1.0, 1.0, 0.8 ) );
                float3 N = normalize( in.normal );
        
                half NdotL = half( saturate( dot( N, L ) ) );
        
                half3 ambient = ( in.color * texel * 0.1 );
                half3 diffuse = ( in.color * texel * NdotL );
                half3 illum = ambient + diffuse;
        
                return half4( illum, 1.0 );
            }
        )" );
        
        NS::Error* pError = nullptr;
        auto pLibrary = NS::TransferPtr( _pDevice->newLibrary( shaderSource, nullptr, &pError ) );
        if ( !pLibrary )
        {
            __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
            assert( false );
        }
        
        auto pVertexMainFn = NS::TransferPtr( pLibrary->newFunction( CreateUTF8String( "vertexMain" ) ) );
        auto pFragmentMainFn = NS::TransferPtr( pLibrary->newFunction( CreateUTF8String( "fragmentMain" ) ) );
        
        auto pRenderPipelineDesc = NS::TransferPtr( MTL::RenderPipelineDescriptor::alloc()->init() );
        pRenderPipelineDesc->setVertexFunction( pVertexMainFn.get() );
        pRenderPipelineDesc->setFragmentFunction( pFragmentMainFn.get() );
        pRenderPipelineDesc->colorAttachments()->object( 0 )->setPixelFormat( MTL::PixelFormatBGRA8Unorm_sRGB );
        pRenderPipelineDesc->setDepthAttachmentPixelFormat( MTL::PixelFormat::PixelFormatDepth16Unorm );
        
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
        constexpr float s = 0.5f;
        
        VertexData verts[] =
        {
            //                                         Texture
            //   Positions           Normals         Coordinates
            { { -s, -s, +s }, {  0.f,  0.f,  1.f }, { 0.f, 1.f } },
            { { +s, -s, +s }, {  0.f,  0.f,  1.f }, { 1.f, 1.f } },
            { { +s, +s, +s }, {  0.f,  0.f,  1.f }, { 1.f, 0.f } },
            { { -s, +s, +s }, {  0.f,  0.f,  1.f }, { 0.f, 0.f } },

            { { +s, -s, +s }, {  1.f,  0.f,  0.f }, { 0.f, 1.f } },
            { { +s, -s, -s }, {  1.f,  0.f,  0.f }, { 1.f, 1.f } },
            { { +s, +s, -s }, {  1.f,  0.f,  0.f }, { 1.f, 0.f } },
            { { +s, +s, +s }, {  1.f,  0.f,  0.f }, { 0.f, 0.f } },

            { { +s, -s, -s }, {  0.f,  0.f, -1.f }, { 0.f, 1.f } },
            { { -s, -s, -s }, {  0.f,  0.f, -1.f }, { 1.f, 1.f } },
            { { -s, +s, -s }, {  0.f,  0.f, -1.f }, { 1.f, 0.f } },
            { { +s, +s, -s }, {  0.f,  0.f, -1.f }, { 0.f, 0.f } },

            { { -s, -s, -s }, { -1.f,  0.f,  0.f }, { 0.f, 1.f } },
            { { -s, -s, +s }, { -1.f,  0.f,  0.f }, { 1.f, 1.f } },
            { { -s, +s, +s }, { -1.f,  0.f,  0.f }, { 1.f, 0.f } },
            { { -s, +s, -s }, { -1.f,  0.f,  0.f }, { 0.f, 0.f } },

            { { -s, +s, +s }, {  0.f,  1.f,  0.f }, { 0.f, 1.f } },
            { { +s, +s, +s }, {  0.f,  1.f,  0.f }, { 1.f, 1.f } },
            { { +s, +s, -s }, {  0.f,  1.f,  0.f }, { 1.f, 0.f } },
            { { -s, +s, -s }, {  0.f,  1.f,  0.f }, { 0.f, 0.f } },

            { { -s, -s, -s }, {  0.f, -1.f,  0.f }, { 0.f, 1.f } },
            { { +s, -s, -s }, {  0.f, -1.f,  0.f }, { 1.f, 1.f } },
            { { +s, -s, +s }, {  0.f, -1.f,  0.f }, { 1.f, 0.f } },
            { { -s, -s, +s }, {  0.f, -1.f,  0.f }, { 0.f, 0.f } }
        };
        
        uint16_t indices[] = {
             0,  1,  2,  2,  3,  0, /* front */
             4,  5,  6,  6,  7,  4, /* right */
             8,  9, 10, 10, 11,  8, /* back */
            12, 13, 14, 14, 15, 12, /* left */
            16, 17, 18, 18, 19, 16, /* top */
            20, 21, 22, 22, 23, 20, /* bottom */
        };
        
        constexpr size_t vertexDataSize = sizeof( verts );
        constexpr size_t indexDataSize = sizeof( indices );
        
        _pVertexDataBuffer = _pDevice->newBuffer( vertexDataSize, MTL::ResourceStorageModeManaged );
        _pIndexBuffer = _pDevice->newBuffer( indexDataSize, MTL::ResourceStorageModeManaged );
        
        memcpy( _pVertexDataBuffer->contents(), verts, vertexDataSize );
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
        
        uint8_t* pTextureData = ( uint8_t* )alloca( DEFAULT_TEXTURE_WIDTH * DEFAULT_TEXTURE_HEIGHT * 4 );
        for ( size_t y = 0; y < DEFAULT_TEXTURE_HEIGHT; ++y )
        {
            for ( size_t x = 0; x < DEFAULT_TEXTURE_WIDTH; ++x )
            {
                bool isWhite = ( x ^ y ) & 0b1000000;
                uint8_t c = isWhite ? 0xFF : 0xA;
                
                size_t i = y * DEFAULT_TEXTURE_WIDTH + x;
                
                pTextureData[ i * 4 + 0 ] = c;
                pTextureData[ i * 4 + 1 ] = c;
                pTextureData[ i * 4 + 2 ] = c;
                pTextureData[ i * 4 + 3 ] = 0xFF;
            }
        }
        
        _pTexture->replaceRegion( MTL::Region( 0, 0, 0, DEFAULT_TEXTURE_WIDTH, DEFAULT_TEXTURE_HEIGHT, 1 ), 0, pTextureData, DEFAULT_TEXTURE_WIDTH * 4 );
    }
    
}
