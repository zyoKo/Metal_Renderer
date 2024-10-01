//
//  AssetRepository.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#ifndef AssetRepository_hpp
#define AssetRepository_hpp

#include <unordered_map>
#include <memory>

namespace NS
{
    class String;
}

namespace PCR
{
    class Shader;
}

namespace PCR
{
    class AssetRepository
    {
    public:
        std::weak_ptr<Shader> GetShaderByName(NS::String* shaderName);
        
    private:
        std::unordered_map<NS::String*, std::shared_ptr<Shader>> shaders;
    };
}

#endif /* AssetRepository_hpp */
