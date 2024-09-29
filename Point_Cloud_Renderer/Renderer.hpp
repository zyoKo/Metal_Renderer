//
//  Renderer.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#pragma once

#include <MetalKit/MetalKit.hpp>

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
        MTL::Buffer* _pVertexPositionBuffer;
        MTL::Buffer* _pVertexColorBuffer;
        
        void BuildShaders();
        void BuildBuffers();
    };
}
