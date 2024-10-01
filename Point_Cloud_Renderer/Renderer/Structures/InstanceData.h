//
//  InstanceData.h
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/1/24.
//

#ifndef InstanceData_h
#define InstanceData_h

namespace PCR
{
    struct InstanceData
    {
        simd::float4x4 transform;
        simd::float4 color;
    };
}

#endif /* InstanceData_h */
