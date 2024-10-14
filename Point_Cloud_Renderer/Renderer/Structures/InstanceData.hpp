//
//  InstanceData.h
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/1/24.
//

#ifndef InstanceData_hpp
#define InstanceData_hpp

namespace PCR
{
    struct InstanceData
    {
        simd::float4x4 transform;
        
        simd::float3x3 normalTransform;
        
        simd::float4 color;
    };
}

#endif /* InstanceData_hpp */
