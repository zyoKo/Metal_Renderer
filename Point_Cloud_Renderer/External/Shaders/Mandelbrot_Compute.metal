//
//  Mandelbrot_Compute.metal
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/11/24.
//

#include <metal_stdlib>
using namespace metal;

kernel void mandelbrot_set( texture2d< half, access::write > tex [[ texture(0) ]],
                            uint2 index [[thread_position_in_grid]],
                            uint2 gridSize [[threads_per_grid]],
                            device const uint* frame [[ buffer(0) ]] )
{
    constexpr float ANIMATION_FREQUENCY{ 0.01 };
    constexpr float ANIMATION_SPEED{ 4 };
    constexpr float ANIMATION_SCALE_LOW{ 0.62 };
    constexpr float ANIMATION_SCALE{ 0.38 };
    
    constexpr float2 MB_PIXEL_OFFSET = float2{ -0.2, -0.35 };
    constexpr float2 MB_ORIGIN = float2{ -1.2, -0.32 };
    constexpr float2 MB_SCALE = float2{ 2.2, 2.0 };
    
    float zoom = ANIMATION_SCALE_LOW + ANIMATION_SCALE * cos( ANIMATION_FREQUENCY * ( *frame ) );
    zoom = pow( zoom, ANIMATION_SPEED );
    
    // Scale
    float x0 = zoom * MB_SCALE.x * ( (float)index.x / gridSize.x + MB_PIXEL_OFFSET.x ) + MB_ORIGIN.x;
    float y0 = zoom * MB_SCALE.y * ( (float)index.y / gridSize.y + MB_PIXEL_OFFSET.y ) + MB_ORIGIN.y;
    
    // Implement Mandelbrot set
    float x = 0.0;
    float y = 0.0;
    uint iteration = 0;
    uint max_iteration = 1000;
    float xtmp = 0.0;
    
    while ( x * x + y * y <= 4 && iteration < max_iteration )
    {
        xtmp = x * x - y * y + x0;
        y = 2 * x * y + y0;
        x = xtmp;
        
        ++iteration;
    }
    
    // Convert iteration result to colors
    half color = ( 0.5 + 0.5 * cos( 3.0 + iteration * 0.15 ) );
    tex.write( half4( color, color, color, 1.0 ), index, 0 );
}
