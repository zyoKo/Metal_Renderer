//
//  Constants.h
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/30/24.
//

#ifndef Constants_h
#define Constants_h

namespace PCR
{
    constexpr int MAX_FRAMES_IN_FLIGHT{ 3 };
    
    constexpr int INSTANCE_ROWS{ 10 };
    constexpr int INSTANCE_COLUMNS{ 10 };
    constexpr int INSTANCE_DEPTH{ 10 };
    constexpr size_t MAX_NUM_INSTANCES{ INSTANCE_ROWS * INSTANCE_COLUMNS * INSTANCE_DEPTH };
    
    constexpr uint32_t DEFAULT_TEXTURE_WIDTH{ 128 };
    constexpr uint32_t DEFAULT_TEXTURE_HEIGHT{ 128 };
}

#endif /* Constants_h */
