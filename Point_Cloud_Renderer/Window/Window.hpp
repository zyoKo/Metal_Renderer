//
//  Window.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#ifndef Window_hpp
#define Window_hpp

#include "Structures/WindowProperties.hpp"

FD_MTK

FD_NS

FD_MTL

namespace PCR
{
    class Window
    {
    public:
        Window( WindowProperties& windowProp, MTL::Device* pDevice );
        
        explicit Window( CGRect windowDimensions, MTL::Device* pDevice );
        
        void create( MTK::View* pMTKView );
        
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
