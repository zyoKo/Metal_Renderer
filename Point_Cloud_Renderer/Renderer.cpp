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
    Renderer::Renderer( MTL::Device* pDevice )
    :   _pDevice( pDevice->retain() )
    ,   _frame( 0 )
    ,   _angle( 0.0f )
    {
        _pCommandQueue = _pDevice->newCommandQueue();
        buildShaders();
        buildBuffers();
        buildFrameData();
        
        _semaphore = dispatch_semaphore_create( MAX_FRAMES_IN_FLIGHT );
    }

    Renderer::~Renderer()
    {
        _pShaderLibrary->release();
        _pArgBuffer->release();
        _pVertexColorBuffer->release();
        _pVertexPositionBuffer->release();
        for ( int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
        {
            _pFrameData[ i ]->release();
        }
        _pRenderPipelineStateObject->release();
        _pCommandQueue->release();
        _pDevice->release();
    }

    void Renderer::draw( MTK::View* pView )
    {
        NS::AutoreleasePool* pPool = NS::AutoreleasePool::alloc()->init();
        
        _frame = (_frame + 1) % MAX_FRAMES_IN_FLIGHT;
        MTL::Buffer* pCurrentFrameBuffer = _pFrameData[ _frame ];

        MTL::CommandBuffer* pCmd = _pCommandQueue->commandBuffer();
        dispatch_semaphore_wait( _semaphore, DISPATCH_TIME_FOREVER );
        pCmd->addCompletedHandler( ^void( MTL::CommandBuffer* pCmd ){
            dispatch_semaphore_signal( this->_semaphore );
        });
        
        reinterpret_cast< FrameData* >( pCurrentFrameBuffer->contents() )->angle = ( _angle += 0.01f );
        pCurrentFrameBuffer->didModifyRange( NS::Range::Make( 0, sizeof( FrameData ) ) );
        
        MTL::RenderPassDescriptor* pRpd = pView->currentRenderPassDescriptor();
        MTL::RenderCommandEncoder* pEnc = pCmd->renderCommandEncoder( pRpd );
        
        pEnc->setRenderPipelineState( _pRenderPipelineStateObject );
        pEnc->setVertexBuffer( _pArgBuffer, 0, 0 );
        pEnc->useResource( _pVertexPositionBuffer, MTL::ResourceUsageRead );
        pEnc->useResource( _pVertexColorBuffer, MTL::ResourceUsageRead );
        
        pEnc->setVertexBuffer( pCurrentFrameBuffer, 0, 1 );
        
        // Draw-call
        pEnc->drawPrimitives( MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger( 0 ), NS::UInteger( 3 ), NS::UInteger( 1 ) );
        
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
                device float3* positions [[id(0)]];
                device float3* colors [[id(1)]];
            };
        
            struct FrameData
            {
                float angle;
            };

            v2f vertex vertexMain( uint vertexId [[vertex_id]],
                                   device const VertexData* vertexData [[buffer(0)]],
                                   constant FrameData* frameData [[buffer(1)]] )
            {
                float a = frameData->angle;
                float3x3 rotationMatrix = float3x3( sin( a ),  cos( a ), 0.0,
                                                    cos( a ), -sin( a ), 0.0,
                                                    0.0     , 0.0      , 1.0 );
                v2f o;
                o.position = float4( rotationMatrix * vertexData->positions[ vertexId ], 1.0 );
                o.color = half3 ( vertexData->colors[ vertexId ] );
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
        constexpr size_t NumVertices = 3;

        constexpr simd::float3 positions[NumVertices] =
        {
            { -0.8f,  0.8f, 0.0f },
            {  0.0f, -0.8f, 0.0f },
            {  0.8f,  0.8f, 0.0f }
        };

        constexpr simd::float3 colors[NumVertices] =
        {
            {  1.0f, 0.3f, 0.2f },
            {  0.8f, 1.0f, 0.0f },
            {  0.8f, 0.0f, 1.0f }
        };
        
        constexpr size_t positionDataSize = NumVertices * sizeof( simd::float3 );
        constexpr size_t colorDataSize = NumVertices * sizeof( simd::float3 );
        
        _pVertexPositionBuffer = _pDevice->newBuffer( positionDataSize, MTL::ResourceStorageModeManaged );
        _pVertexColorBuffer = _pDevice->newBuffer( colorDataSize, MTL::ResourceStorageModeManaged );
        
        memcpy( _pVertexPositionBuffer->contents(), positions, positionDataSize );
        memcpy( _pVertexColorBuffer->contents(), colors, colorDataSize );
        
        _pVertexPositionBuffer->didModifyRange( NS::Range::Make( 0, _pVertexPositionBuffer->length() ) );
        _pVertexColorBuffer->didModifyRange( NS::Range::Make( 0, _pVertexColorBuffer->length() ) );
        
        assert(_pShaderLibrary);
        
        MTL::Function* pVertexFn = _pShaderLibrary->newFunction( NS::String::string( "vertexMain", NS::UTF8StringEncoding ) );
        MTL::ArgumentEncoder* pArgEncoder = pVertexFn->newArgumentEncoder( 0 );
        
        MTL::Buffer* pArgBuffer = _pDevice->newBuffer( pArgEncoder->encodedLength(), MTL::ResourceStorageModeManaged );
        _pArgBuffer = pArgBuffer;
        
        pArgEncoder->setArgumentBuffer( _pArgBuffer, 0 );
        pArgEncoder->setBuffer( _pVertexPositionBuffer, 0, 0 );
        pArgEncoder->setBuffer( _pVertexColorBuffer, 0, 1 );
        
        _pArgBuffer->didModifyRange( NS::Range::Make( 0, _pArgBuffer->length() ) );
        
        pVertexFn->release();
        pArgEncoder->release();
    }
    
    void Renderer::buildFrameData()
    {
        for ( int i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
        {
            _pFrameData[ i ] = _pDevice->newBuffer( sizeof( FrameData ), MTL::ResourceStorageModeManaged );
        }
    }
}
