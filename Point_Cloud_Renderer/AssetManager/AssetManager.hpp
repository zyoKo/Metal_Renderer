//
//  AssetManager.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#ifndef AssetManager_hpp
#define AssetManager_hpp

#include <memory>

namespace PCR
{
    class AssetRepository;
}

namespace PCR
{
    class AssetManager
    {
    public:
        AssetManager();
        
        void Clear();
        
    private:
        std::shared_ptr<AssetRepository> assetRepo;
    };
}

#endif /* AssetManager_hpp */
