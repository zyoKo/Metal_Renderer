//
//  Utility.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/1/24.
//

#ifndef Utility_hpp
#define Utility_hpp

#include <simd/simd.h>

namespace PCR::Math
{
    static constexpr simd::float4x4 makeIdentity()
    {
        return {
            simd::float4{ 1.0f, 0.0f, 0.0f, 0.0f },
            simd::float4{ 0.0f, 1.0f, 0.0f, 0.0f },
            simd::float4{ 0.0f, 0.0f, 1.0f, 0.0f },
            simd::float4{ 0.0f, 0.0f, 0.0f, 1.0f }
        };
    }
    
    static simd::float4x4 makePerspective( float fovRadians, float aspect, float zNear, float zFar )
    {
        float ys = 1.f / tanf(fovRadians * 0.5f);
        float xs = ys / aspect;
        float zs = zFar / ( zNear - zFar );
        return simd_matrix_from_rows((simd::float4){ xs,   0.0f, 0.0f,  0.0f },
                                     (simd::float4){ 0.0f, ys,   0.0f,  0.0f },
                                     (simd::float4){ 0.0f, 0.0f, zs,    zNear * zs },
                                     (simd::float4){ 0.0f, 0.0f, -1.0f, 0.0f });
    }
    
    static simd::float4x4 makeXRotate( float angleRadians )
    {
        const float a = angleRadians;
        return simd_matrix_from_rows((simd::float4){ 1.0f, 0.0f, 0.0f, 0.0f },
                                     (simd::float4){ 0.0f, cosf( a ), sinf( a ), 0.0f },
                                     (simd::float4){ 0.0f, -sinf( a ), cosf( a ), 0.0f },
                                     (simd::float4){ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    static simd::float4x4 makeYRotate( float angleRadians )
    {
        const float a = angleRadians;
        return simd_matrix_from_rows((simd::float4){ cosf( a ), 0.0f, sinf( a ), 0.0f },
                                     (simd::float4){ 0.0f, 1.0f, 0.0f, 0.0f },
                                     (simd::float4){ -sinf( a ), 0.0f, cosf( a ), 0.0f },
                                     (simd::float4){ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    static simd::float4x4 makeZRotate( float angleRadians )
    {
        const float a = angleRadians;
        return simd_matrix_from_rows((simd::float4){ cosf( a ), sinf( a ), 0.0f, 0.0f },
                                     (simd::float4){ -sinf( a ), cosf( a ), 0.0f, 0.0f },
                                     (simd::float4){ 0.0f, 0.0f, 1.0f, 0.0f },
                                     (simd::float4){ 0.0f, 0.0f, 0.0f, 1.0f });
    }

    static simd::float4x4 makeTranslate( const simd::float3& v )
    {
        const simd::float4 col0 = { 1.0f, 0.0f, 0.0f, 0.0f };
        const simd::float4 col1 = { 0.0f, 1.0f, 0.0f, 0.0f };
        const simd::float4 col2 = { 0.0f, 0.0f, 1.0f, 0.0f };
        const simd::float4 col3 = { v.x, v.y, v.z, 1.0f };
        return simd_matrix( col0, col1, col2, col3 );
    }

    static simd::float4x4 makeScale( const simd::float3& v )
    {
        return simd_matrix((simd::float4){ v.x, 0, 0, 0 },
                           (simd::float4){ 0, v.y, 0, 0 },
                           (simd::float4){ 0, 0, v.z, 0 },
                           (simd::float4){ 0, 0, 0, 1.0 });
    }
    
    static simd::float3x3 discardTranslation( const simd::float4x4& m )
    {
        return simd_matrix( m.columns[0].xyz, m.columns[1].xyz, m.columns[2].xyz );
    }
}

#endif /* Utility_hpp */
