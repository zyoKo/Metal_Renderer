//
//  Renderer.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#pragma once

#include "Core/Core.h"
#include "Renderer/Data/Constants.h"

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
        
        MTL::Buffer* _pVertexDataBuffer;
        MTL::Buffer* _pInstanceDataBuffers[ MAX_FRAMES_IN_FLIGHT ];
        MTL::Buffer* _pCameraDataBuffers[ MAX_FRAMES_IN_FLIGHT ];
        MTL::Buffer* _pIndexBuffer;
        
        int _frame;
        float _angle;
        
        dispatch_semaphore_t _semaphore;
        
        void buildShaders();
        
        void buildBuffers();
        
        void buildDepthStencilStates();
    };
}
