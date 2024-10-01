//
//  MyMTKViewDelegate.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#pragma once

#include <MetalKit/MetalKit.hpp>

// Forward Declerations
namespace PCR
{
    class Renderer;
}

namespace PCR
{
    class MyMTKViewDelegate : public MTK::ViewDelegate
    {
        public:
            MyMTKViewDelegate( MTL::Device* pDevice );
        
            virtual ~MyMTKViewDelegate() override;
        
            virtual void drawInMTKView( MTK::View* pView ) override;

        private:
            Renderer* _pRenderer;
    };
}
