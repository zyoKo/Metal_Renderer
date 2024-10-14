//
//  SubMesh.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#ifndef SubMesh_hpp
#define SubMesh_hpp

#include <array>

#include <MetalKit/MetalKit.hpp>

#include "Renderer/Buffer/MeshBuffer.hpp"
#include "Renderer/Mesh/Data/Constants.hpp"

namespace PCR
{
    using SubmeshTextureArray = std::array< MTL::Texture*, SUBMESH_TEXTURE_COUNT >;
    
    class SubMesh
    {
    public:
        SubMesh();
        
        SubMesh( MTL::PrimitiveType primitiveType,
                 MTL::IndexType indexType,
                 NS::UInteger indexCount,
                 const MeshBuffer& indexBuffer,
                 const SubmeshTextureArray& pTextures );
        
        SubMesh( MTL::PrimitiveType primitiveType,
                 MTL::IndexType indexType,
                 NS::UInteger indexCount,
                 const MeshBuffer& indexBuffer );
        
        SubMesh( const SubMesh& rhs );
        
        SubMesh( SubMesh&& rhs );
        
        SubMesh& operator=( const SubMesh& rhs );
        
        SubMesh& operator=( SubMesh&& rhs );
        
        ~SubMesh();
        
        MTL::PrimitiveType GetPrimitiveType() const;
        
        MTL::IndexType GetIndexType() const;
        
        NS::UInteger GetIndexCount() const;
        
        const MeshBuffer& GetMeshBuffer() const;
        
        const SubmeshTextureArray& GetTextures() const;
        
    private:
        MTL::PrimitiveType primitiveType;
        
        MTL::IndexType indexType;
        
        NS::UInteger indexCount;
        
        MeshBuffer indexBuffer;
        
        SubmeshTextureArray pTextures;
    };
}

#endif /* SubMesh_hpp */
