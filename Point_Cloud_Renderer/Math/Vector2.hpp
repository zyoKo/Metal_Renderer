//
//  Vector2.h
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/30/24.
//

#ifndef Vector2_hpp
#define Vector2_hpp

template <typename T>
struct Vector2
{
    union
    {
        T data[2];
        
        struct
        {
            T x;
            T y;
        };
        
        struct
        {
            T width;
            T height;
        };
    };
};

using Vec2F = Vector2<float>;
using Vector2F = Vector2<float>;

#endif /* Vector2_hpp */
