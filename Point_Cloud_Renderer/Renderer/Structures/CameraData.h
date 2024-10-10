//
//  CameraData.h
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/1/24.
//

#ifndef CameraData_h
#define CameraData_h

namespace PCR
{
    struct CameraData
    {
        simd::float4x4 perspectiveTransform;
        simd::float4x4 worldTransform;
        simd::float3x3 worldNormalTransform;
    };
}

#endif /* CameraData_h */
