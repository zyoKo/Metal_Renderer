//
//  MyMTKViewDelegate.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#include "MyMTKViewDelegate.hpp"

#include "Renderer/Renderer.hpp"

namespace PCR
{
    MyMTKViewDelegate::MyMTKViewDelegate( MTL::Device* pDevice )
        : MTK::ViewDelegate()
        , _pRenderer( new Renderer( pDevice ) )
    { }

    MyMTKViewDelegate::~MyMTKViewDelegate()
    {
        delete _pRenderer;
    }

    void MyMTKViewDelegate::drawInMTKView( MTK::View* pView )
    {
        _pRenderer->draw( pView );
    }
}
