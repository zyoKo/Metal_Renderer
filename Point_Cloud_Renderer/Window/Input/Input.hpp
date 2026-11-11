#ifndef Input_hpp
#define Input_hpp

#include <cstdint>

namespace PCR
{
    // macOS virtual key codes (kVK_ANSI_*). Num0-Num9 are the number row, not the keypad.
    enum class Key : uint16_t
    {
        A = 0x00, S = 0x01, D = 0x02, W = 0x0D, Q = 0x0C, E = 0x0E,
        
        Num1 = 0x12, Num2 = 0x13, Num3 = 0x14, Num4 = 0x15, Num5 = 0x17,
        Num6 = 0x16, Num7 = 0x1A, Num8 = 0x1C, Num9 = 0x19, Num0 = 0x1D,
        
        Space  = 0x31,
        Escape = 0x35,
    };
    
    class Input final
    {
    public:
        Input() = delete;
        
        static void initialize();
        
        static void shutdown();
        
        static bool isKeyDown( Key key );
        
        static bool wasKeyPressed( Key key );
        
        static bool wasKeyReleased( Key key );
        
        // Call once at the end of every frame.
        static void endFrame();
        
        // Num1..Num9, Num0 -> 0..9, or -1 if none was pressed this frame.
        static int numberKeyPressed();
    };
}

#endif /* Input_hpp */
