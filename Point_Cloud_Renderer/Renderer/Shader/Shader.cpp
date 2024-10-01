//
//  Shader.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#include "Shader.hpp"

#include "Core/Core.hpp"

namespace PCR
{
    Shader::Shader( MTL::Device* ptr )
    :   _pDevice(ptr)
    { }
    
    void Shader::Create( NS::String* shaderSourcePath )
    {
        NS::String* shaderSource = ParseShader( shaderSourcePath );
        
        NS::Error* pError = nullptr;
        MTL::Library* pLibrary = _pDevice->newLibrary( shaderSource, nullptr, &pError );
        if ( !pLibrary )
        {
            __builtin_printf( "%s", pError->localizedDescription()->utf8String() );
            assert( false );
        }
        
        pVertexMainFn = pLibrary->newFunction( CreateUTF8String("vertexMain") );
        pFragmentMainFn = pLibrary->newFunction( CreateUTF8String("fragmentMain") );
    }
    
    std::vector<MTL::Function*> Shader::GetShaderFunctions() const
    {
        return { pVertexMainFn, pFragmentMainFn };
    }
    
    void Shader::Clear() const
    {
        pVertexMainFn->release();
        pFragmentMainFn->release();
    }
    
    NS::String* Shader::ParseShader(NS::String *shaderSourcePath)
    {
        return CreateUTF8String( R"(
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
    }
}
