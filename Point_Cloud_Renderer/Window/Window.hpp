//
//  Window.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#ifndef Window_hpp
#define Window_hpp

#include <array>

#include <MetalKit/MetalKit.hpp>

#include "Math/Math.h"

namespace PCR
{
    struct WindowProperties
    {
        Vec2F dimension;
        
        Vec2F origin;
        
        NS::String* windowName = NS::String::string( "New Window", NS::UTF8StringEncoding );
        
        CGRect operator()() const
        {
            CGRect frame{ { dimension.x, dimension.y }, { origin.x, origin.y } };
            return frame;
        }
        
        void operator()( const CGRect& frame )
        {
            dimension.width = frame.size.width;
            dimension.height = frame.size.height;
            origin.x = frame.origin.x;
            origin.y = frame.origin.y;
        }
    };
    
    class Window
    {
    public:
        Window( WindowProperties& windowProp, MTL::Device* pDevice );
        
        explicit Window( CGRect windowDimensions, MTL::Device* pDevice );
        
        bool create( MTK::View* pMTKView );
        
        void release();
        
        const WindowProperties& getWindowProperties() const;
        
        WindowProperties& getWindowProperties();
        
        NS::Window* getWindow() const;
        
    private:
        NS::Window* _pWindow;
        
        MTL::Device* _pDevice;
        
        WindowProperties windowProperties;
    };
}

#endif /* Window_hpp */
