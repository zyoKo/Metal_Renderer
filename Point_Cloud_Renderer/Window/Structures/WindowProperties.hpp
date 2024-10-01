//
//  WindowProperties.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/1/24.
//

#ifndef WindowProperties_hpp
#define WindowProperties_hpp

#include "Math/Vector2.h"
#include "Core/Core.h"

FD_NS

namespace PCR
{
    struct WindowProperties
    {
        Vec2F dimension;
        
        Vec2F origin;
        
        NS::String* windowName = NS::String::string( "New Window", NS::UTF8StringEncoding );
        
        explicit WindowProperties( const CGRect& frame, NS::String* winName = CreateUTF8String( "New Window" ) );
        
        CGRect operator()() const;
        
        void operator()( const CGRect& frame );
    };
}

#endif /* WindowProperties_hpp */
