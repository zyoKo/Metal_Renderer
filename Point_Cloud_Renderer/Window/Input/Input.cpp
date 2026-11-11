#include "Input.hpp"

#include <array>

#include <objc/message.h>
#include <objc/runtime.h>

namespace PCR
{
    namespace
    {
        // Values from NSEvent.h; metal-cpp doesn't wrap NSEvent.
        constexpr unsigned long long EVENT_MASK_KEY_DOWN{ 1ull << 10 };
        constexpr unsigned long long EVENT_MASK_KEY_UP{ 1ull << 11 };
        constexpr unsigned long EVENT_TYPE_KEY_DOWN{ 10 };
        constexpr unsigned long MODIFIER_FLAG_COMMAND{ 1ul << 20 };
        
        constexpr size_t KEY_COUNT{ 128 };
        
        std::array< bool, KEY_COUNT > s_keyDown{};
        std::array< bool, KEY_COUNT > s_keyPressed{};
        std::array< bool, KEY_COUNT > s_keyReleased{};
        
        id s_pMonitor{ nil };
        
        template< typename Ret, typename... Args >
        Ret sendMessage( id receiver, const char* selector, Args... args )
        {
            using Fn = Ret (*)( id, SEL, Args... );
            return reinterpret_cast< Fn >( objc_msgSend )( receiver, sel_registerName( selector ), args... );
        }
        
        id handleKeyEvent( id pEvent )
        {
            // Leave Cmd shortcuts to the menu bar.
            const auto modifiers = sendMessage< unsigned long >( pEvent, "modifierFlags" );
            if ( modifiers & MODIFIER_FLAG_COMMAND )
            {
                return pEvent;
            }
            
            const auto keyCode = sendMessage< unsigned short >( pEvent, "keyCode" );
            if ( keyCode < KEY_COUNT )
            {
                const bool isKeyDownEvent = sendMessage< unsigned long >( pEvent, "type" ) == EVENT_TYPE_KEY_DOWN;
                
                if ( isKeyDownEvent )
                {
                    // Ignore key-repeat.
                    if ( !s_keyDown[ keyCode ] )
                    {
                        s_keyPressed[ keyCode ] = true;
                    }
                    s_keyDown[ keyCode ] = true;
                }
                else
                {
                    s_keyDown[ keyCode ] = false;
                    s_keyReleased[ keyCode ] = true;
                }
            }
            
            // Returning nil swallows the event so the window doesn't beep.
            return nil;
        }
        
        size_t indexOf( Key key )
        {
            return static_cast< size_t >( key );
        }
    }
    
    void Input::initialize()
    {
        if ( s_pMonitor )
        {
            return;
        }
        
        id (^handler)( id ) = ^id( id pEvent ) { return handleKeyEvent( pEvent ); };
        
        id nsEventClass = reinterpret_cast< id >( objc_getClass( "NSEvent" ) );
        id pMonitor = sendMessage< id >( nsEventClass,
                                         "addLocalMonitorForEventsMatchingMask:handler:",
                                         EVENT_MASK_KEY_DOWN | EVENT_MASK_KEY_UP,
                                         handler );
        
        s_pMonitor = sendMessage< id >( pMonitor, "retain" );
    }
    
    void Input::shutdown()
    {
        if ( s_pMonitor )
        {
            id nsEventClass = reinterpret_cast< id >( objc_getClass( "NSEvent" ) );
            sendMessage< void >( nsEventClass, "removeMonitor:", s_pMonitor );
            sendMessage< void >( s_pMonitor, "release" );
            s_pMonitor = nil;
        }
        
        s_keyDown.fill( false );
        s_keyPressed.fill( false );
        s_keyReleased.fill( false );
    }
    
    bool Input::isKeyDown( Key key )
    {
        return s_keyDown[ indexOf( key ) ];
    }
    
    bool Input::wasKeyPressed( Key key )
    {
        return s_keyPressed[ indexOf( key ) ];
    }
    
    bool Input::wasKeyReleased( Key key )
    {
        return s_keyReleased[ indexOf( key ) ];
    }
    
    void Input::endFrame()
    {
        s_keyPressed.fill( false );
        s_keyReleased.fill( false );
    }
    
    int Input::numberKeyPressed()
    {
        static constexpr Key numberKeys[] = {
            Key::Num1, Key::Num2, Key::Num3, Key::Num4, Key::Num5,
            Key::Num6, Key::Num7, Key::Num8, Key::Num9, Key::Num0
        };
        
        for ( int i = 0; i < 10; ++i )
        {
            if ( wasKeyPressed( numberKeys[ i ] ) )
            {
                return i;
            }
        }
        
        return -1;
    }
}
