//
//  Renderer.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#include "Renderer.hpp"

#include <cassert>

#include <AppKit/AppKit.hpp>
#include <simd/simd.h>
#include "Renderer/Structures/FrameData.h"

#include "Core/Core.h"

namespace PCR
{
    struct InstanceData
    {
        simd::float4x4 transform;
        simd::float4 color;
    };
}

namespace PCR
{
    Renderer::Renderer( MTL::Device* pDevice )
    :   _pDevice( pDevice->retain() )
    ,   _frame( 0 )
    ,   _angle( 0.0f )
    {
        _pCommandQueue = _pDevice->newCommandQueue();
        buildShaders();
        buildBuffers();
        
        _semaphore = dispatch_semaphore_create( MAX_FRAMES_IN_FLIGHT );
    }

    Renderer::~Renderer()
    {
        _pShaderLibrary->release();
        _pVertexBuffer->release();
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
        
        // Instance Data
        for ( size_t i = 0; i < MAX_NUM_INSTANCES; ++i )
        {
            float iDivNumInstances = i / static_cast<float>( MAX_NUM_INSTANCES );
            float xoff = ( iDivNumInstances * 2.0f - 1.0f ) + ( 1.0f / MAX_NUM_INSTANCES );
            float yoff = sin( ( iDivNumInstances + _angle ) * 2.0f * M_PI );
            
            // Instance Transform
            pInstanceData[ i ].transform = {
                (simd::float4){ scl * sinf( _angle ), scl *  cosf( _angle ), 0.f, 0.f },
                (simd::float4){ scl * cosf( _angle ), scl * -sinf( _angle ), 0.f, 0.f },
                (simd::float4){ 0.f,                  0.f,                   scl, 0.f },
                (simd::float4){ xoff,                 yoff,                  0.f, 1.f } };

            float r = iDivNumInstances;
            float g = 1.0f - r;
            float b = sinf( M_PI * 2.0f * iDivNumInstances );
            
            // Instance Color
            pInstanceData[ i ].color = { r, g, b, 1.0f };
        }
        pCurrentInstanceDataBuffer->didModifyRange( NS::Range::Make( 0, pCurrentInstanceDataBuffer->length() ) );
        
        MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
        MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder( pRpd );
        
        pEnc->setRenderPipelineState( _pRenderPipelineStateObject );
        pEnc->setVertexBuffer( _pVertexBuffer, 0, 0 );
        pEnc->setVertexBuffer( pCurrentInstanceDataBuffer, 0, 1 );
        
        // Draw-call
        pEnc->drawIndexedPrimitives( MTL::PrimitiveType::PrimitiveTypeTriangle,
                                     /* indexCount */ 6,
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

            v2f vertex vertexMain( uint vertexId [[vertex_id]],
                                   uint instanceID [[instance_id]],
                                   device const VertexData* vertexData [[buffer(0)]],
                                   device const InstanceData* instanceData [[buffer(1)]] )
            {
                v2f o;
                float4 pos = float4( vertexData[ vertexId ].position, 1.0 );
                o.position = instanceData[ instanceID ].transform * pos;
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
        
        constexpr simd::float3 vertices[4] = {
            { -s, -s, +s },
            { +s, -s, +s },
            { +s, +s, +s },
            { -s, +s, +s }
        };
        
        constexpr uint16_t indices[6] = {
            0, 1, 2,
            2, 3, 0
        };
        
        constexpr size_t vertexDataSize = sizeof( vertices );
        constexpr size_t indexDataSize = sizeof( indices );
        
        _pVertexBuffer = _pDevice->newBuffer( vertexDataSize, MTL::ResourceStorageModeManaged );
        _pIndexBuffer = _pDevice->newBuffer( indexDataSize, MTL::ResourceStorageModeManaged );
        
        memcpy( _pVertexBuffer->contents(), vertices, vertexDataSize );
        memcpy( _pIndexBuffer->contents(), indices, indexDataSize );
        
        _pVertexBuffer->didModifyRange( NS::Range::Make( 0, _pVertexBuffer->length() ) );
        _pIndexBuffer->didModifyRange( NS::Range::Make( 0, _pIndexBuffer->length() ) );
        
        constexpr size_t instanceDataSize = MAX_FRAMES_IN_FLIGHT * MAX_NUM_INSTANCES * sizeof( InstanceData );
        for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
        {
            _pInstanceDataBuffers[ i ] = _pDevice->newBuffer( instanceDataSize, MTL::ResourceStorageModeManaged );
        }
    }
}
