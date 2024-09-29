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

#include "Core/Core.h"

namespace PCR
{
    Renderer::Renderer( MTL::Device* pDevice )
        : _pDevice( pDevice->retain() )
    {
        _pCommandQueue = _pDevice->newCommandQueue();
        BuildShaders();
        BuildBuffers();
    }

    Renderer::~Renderer()
    {
        _pVertexColorBuffer->release();
        _pVertexPositionBuffer->release();
        _pRenderPipelineStateObject->release();
        _pCommandQueue->release();
        _pDevice->release();
    }

    void Renderer::draw( MTK::View* pView )
    {
        NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();

        MTL::CommandBuffer* pCmd = _pCommandQueue->commandBuffer();
        MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
        MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder( pRpd );
        
        pEnc->setRenderPipelineState( _pRenderPipelineStateObject );
        pEnc->setVertexBuffer( _pVertexPositionBuffer, 0, 0 );
        pEnc->setVertexBuffer( _pVertexColorBuffer, 0, 1 );
        pEnc->drawPrimitives( MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger( 0 ), NS::UInteger( 3 ), NS::UInteger( 1 ) );
        
        pEnc->endEncoding();
        pCmd->presentDrawable( pView->currentDrawable() );
        pCmd->commit();

        pPool->release();
    }
    
    void Renderer::BuildShaders()
    {
        NS::String* shaderSource = CreateUTF8String( R"(
            #include <metal_stdlib>
            using namespace metal;

            struct v2f
            {
                float4 position [[position]];
                half3 color;
            };

            v2f vertex vertexMain( uint vertexId [[vertex_id]],
                                   device const float3* positions [[buffer(0)]],
                                   device const float3* colors [[buffer(1)]] )
            {
                v2f o;
                o.position = float4( positions[ vertexId ], 1.0 );
                o.color = half3 ( colors[ vertexId ] );
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
        pLibrary->release();
    }
    
    void Renderer::BuildBuffers()
    {
        constexpr size_t NumVertices = 3;

        constexpr simd::float3 positions[NumVertices] =
        {
            { -0.8f,  0.8f, 0.0f },
            {  0.0f, -0.8f, 0.0f },
            { +0.8f,  0.8f, 0.0f }
        };

        constexpr simd::float3 colors[NumVertices] =
        {
            {  1.0, 0.3f, 0.2f },
            {  0.8f, 1.0, 0.0f },
            {  0.8f, 0.0f, 1.0 }
        };
        
        constexpr size_t positionDataSize = NumVertices * sizeof( simd::float3 );
        constexpr size_t colorDataSize = NumVertices * sizeof( simd::float3 );
        
        _pVertexPositionBuffer = _pDevice->newBuffer( positionDataSize, MTL::ResourceStorageModeShared );
        _pVertexColorBuffer = _pDevice->newBuffer( colorDataSize, MTL::ResourceStorageModeShared );
        
        memcpy( _pVertexPositionBuffer->contents(), positions, positionDataSize );
        memcpy( _pVertexColorBuffer->contents(), colors, colorDataSize );
        
        //_pVertexPositionBuffer->didModifyRange( NS::Range::Make( 0, _pVertexPositionBuffer->length() ) );
        //_pVertexColorBuffer->didModifyRange( NS::Range::Make( 0, _pVertexColorBuffer->length() ) );
    }
}
