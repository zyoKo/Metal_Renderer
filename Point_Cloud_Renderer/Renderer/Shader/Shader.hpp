//
//  Shader.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#ifndef Shader_hpp
#define Shader_hpp

#include <AppKit/AppKit.hpp>
#include <MetalKit/MetalKit.hpp>

namespace PCR
{
    class Shader
    {
    public:
        Shader( MTL::Device* ptr );
        
        void Create(NS::String* shaderSourcePath);
        
        std::vector<MTL::Function*> GetShaderFunctions() const;
        
        void Clear() const;
        
    private:
        NS::String* ParseShader( NS::String* shaderSourcePath );
        
        MTL::Device* _pDevice;
        
        MTL::Function* pVertexMainFn;
        
        MTL::Function* pFragmentMainFn;
    };
}

#endif /* Shader_hpp */
