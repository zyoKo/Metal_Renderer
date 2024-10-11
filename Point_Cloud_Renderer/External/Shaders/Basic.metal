//
//  Basic.metal
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/10/24.
//

#include <metal_stdlib>
using namespace metal;

struct v2f
{
    float4 position [[position]];
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
