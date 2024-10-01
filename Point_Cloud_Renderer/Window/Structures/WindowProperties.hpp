//
//  WindowProperties.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/1/24.
//

#ifndef WindowProperties_hpp
#define WindowProperties_hpp

#include <Metal/Metal.hpp>

#include "Math/Vector2.h"
#include "Core/Core.hpp"

FD_NS

namespace PCR
{
    struct WindowProperties
    {
        Vec2F dimension;
        
        Vec2F origin;
        
        NS::String* windowName;
        
        explicit WindowProperties( const CGRect& frame, const char* winName = "New Window" );
        
        CGRect operator()() const;
        
        void operator()( const CGRect& frame );
    };
}

#endif /* WindowProperties_hpp */
