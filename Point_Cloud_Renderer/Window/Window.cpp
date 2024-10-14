//
//  Window.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#include "Window.hpp"

#include <MetalKit/MetalKit.hpp>

namespace PCR
{
    Window::Window( WindowProperties& windowProp, MTL::Device* pDevice )
    :   _pDevice{ pDevice }
    ,   windowProperties{ windowProp }
    {
        _pWindow = NS::Window::alloc()->init( windowProperties(),
                                              NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled,
                                              NS::BackingStoreBuffered,
                                              false );
    }
    
    Window::Window( CGRect windowDimensions, MTL::Device* pDevice )
    :   _pDevice{ pDevice }
    ,   windowProperties{ windowDimensions }
    {
        _pWindow = NS::Window::alloc()->init( windowProperties(),
                                              NS::WindowStyleMaskClosable | NS::WindowStyleMaskTitled,
                                              NS::BackingStoreBuffered,
                                              false );
    }
    
    void Window::create( MTK::View* pMTKView )
    {
        _pWindow->setContentView( pMTKView );
        _pWindow->setTitle( windowProperties.windowName );
        _pWindow->makeKeyAndOrderFront( nullptr );
    }
    
    const WindowProperties& Window::getWindowProperties() const
    {
        return windowProperties;
    }
    
    WindowProperties& Window::getWindowProperties()
    {
        return windowProperties;
    }
    
    NS::Window* Window::getWindow() const
    {
        return _pWindow;
    }
    
    void Window::release()
    {
        _pWindow->release();
    }
}
