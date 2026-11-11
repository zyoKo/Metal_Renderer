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

namespace PCR
{
    static constexpr int SUBMESH_TEXTURE_COUNT{ 3 };
    using SubmeshTextureArray = std::array< MTL::Texture*, SUBMESH_TEXTURE_COUNT >;
    
    class SubMesh final
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
        
        MTL::PrimitiveType getPrimitiveType() const;
        
        MTL::IndexType getIndexType() const;
        
        NS::UInteger getIndexCount() const;
        
        const MeshBuffer& getMeshBuffer() const;
        
        const SubmeshTextureArray& getTextures() const;
        
    private:
        MTL::PrimitiveType primitiveType;
        
        MTL::IndexType indexType;
        
        NS::UInteger indexCount;
        
        MeshBuffer indexBuffer;
        
        SubmeshTextureArray pTextures;
    };
}

#endif /* SubMesh_hpp */
