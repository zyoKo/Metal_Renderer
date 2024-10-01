//
//  WindowProperties.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/1/24.
//

#include "WindowProperties.hpp"

#include <Metal/Metal.hpp>

namespace PCR
{
    WindowProperties::WindowProperties( const CGRect& frame, NS::String* winName /* = CreateUTF8String( "New Window" ) */  )
    {
        dimension.width = frame.size.width;
        dimension.height = frame.size.height;
        origin.x = frame.origin.x;
        origin.y = frame.origin.y;
    }
    
    CGRect WindowProperties::operator()() const
    {
        CGRect frame{ { dimension.x, dimension.y }, { origin.x, origin.y } };
        return frame;
    }
    
    void WindowProperties::operator()( const CGRect& frame )
    {
        dimension.width = frame.size.width;
        dimension.height = frame.size.height;
        origin.x = frame.origin.x;
        origin.y = frame.origin.y;
    }
}
