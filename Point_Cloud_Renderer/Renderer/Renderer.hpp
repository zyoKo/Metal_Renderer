//
//  Renderer.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#ifndef Renderer_hpp
#define Renderer_hpp

#include <Metal/Metal.hpp>

#include "Core/Core.hpp"
#include "Renderer/Data/Constants.hpp"

FD_MTL
FD_MTK

namespace PCR
{
    class Renderer
    {
    public:
        Renderer( MTL::Device* pDevice );
        
        ~Renderer();
        
        void draw( MTK::View* pView );

    private:
        MTL::Device* _pDevice;
        
        MTL::CommandQueue* _pCommandQueue;
        
        MTL::RenderPipelineState* _pRenderPipelineStateObject;
        
        MTL::Library* _pShaderLibrary;
        
        MTL::DepthStencilState* _pDepthStencilState;
        
        MTL::Texture* _pTexture;
        
        MTL::ComputePipelineState* _pComputePipelineStateObject;
        
        MTL::Buffer* _pVertexDataBuffer;
        
        MTL::Buffer* _pInstanceDataBuffers[ MAX_FRAMES_IN_FLIGHT ];
        
        MTL::Buffer* _pCameraDataBuffers[ MAX_FRAMES_IN_FLIGHT ];
        
        MTL::Buffer* _pIndexBuffer;
        
        MTL::Buffer* _pTextureAnimationBuffer;
        
        int _frame;
        
        float _angle;
        
        uint _animationIndex;
        
        dispatch_semaphore_t _semaphore;
        
        void buildShaders();
        
        void buildBuffers();
        
        void buildDepthStencilStates();
        
        void buildTextures();
        
        void buildComputePipeline();
        
        void generateMandelbrotTexture( MTL::CommandBuffer* pCommandBuffer );
    };
}

#endif /* Renderer_hpp */
