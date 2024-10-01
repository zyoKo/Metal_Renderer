//
//  Renderer.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#pragma once

#include <MetalKit/MetalKit.hpp>

#include "Renderer/Data/Constants.h"

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
        
        MTL::Buffer* _pArgBuffer;
        MTL::Buffer* _pVertexPositionBuffer;
        MTL::Buffer* _pVertexColorBuffer;
        MTL::Buffer* _pFrameData[MAX_FRAMES_IN_FLIGHT];
        
        int _frame;
        float _angle;
        
        dispatch_semaphore_t _semaphore;
        
        void buildShaders();
        
        void buildBuffers();
        
        void buildFrameData();
    };
}
