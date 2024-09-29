//
//  MyAppDelegate.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#include "MyAppDelegate.hpp"

#include "Core.h"
#include "Structures/MyMTKViewDelegate.hpp"
#include "Constants/ProjectConstants.h"

namespace PCR
{
    MyAppDelegate::~MyAppDelegate()
    {
        _pMtkView->release();
        _pWindow->release();
        _pDevice->release();
        delete _pViewDelegate;
    }

    NS::Menu* MyAppDelegate::createMenuBar()
    {
        NS::Menu* pMainMenu = NS::Menu::alloc()->init();
        NS::MenuItem* pAppMenuItem = NS::MenuItem::alloc()->init();
        NS::Menu* pAppMenu = NS::Menu::alloc()->init( CreateUTF8String("AppName") );

        NS::String* appName = NS::RunningApplication::currentApplication()->localizedName();
        NS::String* quitItemName = CreateUTF8String("Quit")->stringByAppendingString( appName );
        SEL quitCb = NS::MenuItem::registerActionCallback( "appQuit", [](void*,SEL,const NS::Object* pSender){
            auto pApp = NS::Application::sharedApplication();
            pApp->terminate( pSender );
        } );

        NS::MenuItem* pAppQuitItem = pAppMenu->addItem( quitItemName, quitCb, CreateUTF8String("q") );
        pAppQuitItem->setKeyEquivalentModifierMask( NS::EventModifierFlagCommand );
        pAppMenuItem->setSubmenu( pAppMenu );

        NS::MenuItem* pWindowMenuItem = NS::MenuItem::alloc()->init();
        NS::Menu* pWindowMenu = NS::Menu::alloc()->init( CreateUTF8String("Window") );

        SEL closeWindowCb = NS::MenuItem::registerActionCallback( "windowClose", [](void*, SEL, const NS::Object*){
            auto pApp = NS::Application::sharedApplication();
                pApp->windows()->object< NS::Window >(0)->close();
        } );
        NS::MenuItem* pCloseWindowItem = pWindowMenu->addItem( CreateUTF8String("Close Window"), closeWindowCb, CreateUTF8String("w") );
        pCloseWindowItem->setKeyEquivalentModifierMask( NS::EventModifierFlagCommand );

        pWindowMenuItem->setSubmenu( pWindowMenu );

        pMainMenu->addItem( pAppMenuItem );
        pMainMenu->addItem( pWindowMenuItem );

        pAppMenuItem->release();
        pWindowMenuItem->release();
        pAppMenu->release();
        pWindowMenu->release();

        return pMainMenu->autorelease();
    }

    void MyAppDelegate::applicationWillFinishLaunching( NS::Notification* pNotification )
    {
        NS::Menu* pMenu = createMenuBar();
        NS::Application* pApp = reinterpret_cast< NS::Application* >( pNotification->object() );
        pApp->setMainMenu( pMenu );
        pApp->setActivationPolicy( NS::ActivationPolicy::ActivationPolicyRegular );
    }

    void MyAppDelegate::applicationDidFinishLaunching( NS::Notification* pNotification )
    {
        CGRect frame = (CGRect){ {100.0, 100.0}, {640.0, 480.0} };

        _pWindow = NS::Window::alloc()->init(
            frame,
            NS::WindowStyleMaskClosable|NS::WindowStyleMaskTitled,
            NS::BackingStoreBuffered,
            false );

        _pDevice = MTL::CreateSystemDefaultDevice();

        _pMtkView = MTK::View::alloc()->init( frame, _pDevice );
        _pMtkView->setColorPixelFormat( MTL::PixelFormat::PixelFormatBGRA8Unorm_sRGB );
        _pMtkView->setClearColor( MTL::ClearColor::Make( 1.0, 1.0, 1.0, 1.0 ) );

        _pViewDelegate = new MyMTKViewDelegate( _pDevice );
        _pMtkView->setDelegate( _pViewDelegate );

        _pWindow->setContentView( _pMtkView );
        _pWindow->setTitle( CreateUTF8String(WINDOW_NAME) );

        _pWindow->makeKeyAndOrderFront( nullptr );

        NS::Application* pApp = reinterpret_cast< NS::Application* >( pNotification->object() );
        pApp->activateIgnoringOtherApps( true );
    }

    bool MyAppDelegate::applicationShouldTerminateAfterLastWindowClosed( NS::Application* pSender )
    {
        return true;
    }
}
