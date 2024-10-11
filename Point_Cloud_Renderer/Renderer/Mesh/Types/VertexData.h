//
//  VertexData.h
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/10/24.
//

#ifndef VertexData_h
#define VertexData_h

namespace PCR
{
    struct VertexData
    {
        simd::float3 position;
        simd::float3 normal;
        simd::float2 texCoord;
    };
}

#endif /* VertexData_h */
