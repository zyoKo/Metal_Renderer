#ifndef MeshFactory_hpp
#define MeshFactory_hpp

#include <cstdint>
#include <vector>

#include <simd/simd.h>
#include <Metal/Metal.hpp>

#include "Renderer/Mesh/Mesh.hpp"
#include "Renderer/Mesh/Types/VertexData.h"

namespace PCR::MeshFactory
{
    // Positions: float3 (stride 12). Generics: float3 normal + float2 texCoord (stride 20).
    NS::SharedPtr< MTL::VertexDescriptor > makeVertexDescriptor();
    
    Mesh makeMesh( MTL::Device* pDevice,
                   const std::vector< VertexData >& vertices,
                   const std::vector< uint16_t >& indices );
    
    Mesh makeCube( MTL::Device* pDevice, float size = 1.0f );
    
    Mesh makePyramid( MTL::Device* pDevice, float size = 1.0f );
    
    // (slices + 1) * (stacks + 1) must fit in uint16_t.
    Mesh makeSphere( MTL::Device* pDevice,
                     float radius = 0.5f,
                     uint16_t slices = 32,
                     uint16_t stacks = 16 );
}

#endif /* MeshFactory_hpp */
