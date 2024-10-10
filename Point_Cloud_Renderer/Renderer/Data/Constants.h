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
    constexpr int MAX_NUM_INSTANCES{ INSTANCE_ROWS * INSTANCE_COLUMNS * INSTANCE_DEPTH };
}

#endif /* Constants_h */
