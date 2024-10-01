//
//  MeshVertex.h
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#ifndef MeshVertex_h
#define MeshVertex_h

#include <simd/simd.h>

namespace PCR
{
    struct MeshVertex
    {
        vector_float3 position;
        
        vector_float2 texCoord;
        
        vector_float3 normal;
        
        vector_float3 tangent;
        
        vector_float3 bitangent;
    };
}

#endif /* MeshVertex_h */
