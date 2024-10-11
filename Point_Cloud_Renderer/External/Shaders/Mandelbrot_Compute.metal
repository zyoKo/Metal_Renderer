//
//  Mandelbrot_Compute.metal
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/11/24.
//

#include <metal_stdlib>
using namespace metal;

kernel void mandelbrot_set( texture2d< half, access::write> tex [[texture(0)]],
                            uint2 index [[thread_position_in_grid]],
                            uint2 gridSize [[threads_per_grid]] )
{
    // Scale
    float x0 = 2.0 * index.x / gridSize.x - 1.5;
    float y0 = 2.0 * index.y / gridSize.y - 1.0;
    
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
