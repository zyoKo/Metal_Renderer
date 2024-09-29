//
//  MyAppDelegate.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#pragma once

#include <MetalKit/MetalKit.hpp>

// Point-Cloud-Renderer
namespace PCR
{
    class MyMTKViewDelegate;

    class MyAppDelegate : public NS::ApplicationDelegate
    {
        public:
            ~MyAppDelegate();

            NS::Menu* createMenuBar();

            virtual void applicationWillFinishLaunching( NS::Notification* pNotification ) override;
            virtual void applicationDidFinishLaunching( NS::Notification* pNotification ) override;
            virtual bool applicationShouldTerminateAfterLastWindowClosed( NS::Application* pSender ) override;

        private:
            NS::Window* _pWindow;
            MTK::View* _pMtkView;
            MTL::Device* _pDevice;
            MyMTKViewDelegate* _pViewDelegate = nullptr;
    };
}

