//
//  SubMesh.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#include "SubMesh.hpp"

#include "Renderer/Buffer/MeshBuffer.hpp"

namespace PCR
{
    SubMesh::SubMesh()
    :   primitiveType( MTL::PrimitiveTypeTriangle )
    ,   indexType( MTL::IndexTypeUInt16 )
    ,   indexCount( 0 )
    ,   indexBuffer( nullptr, (NS::UInteger)0, (NS::UInteger)0)
    { }
    
    SubMesh::SubMesh(MTL::PrimitiveType primitiveType,
            MTL::IndexType indexType,
            NS::UInteger indexCount,
            const MeshBuffer& indexBuffer,
            const SubmeshTextureArray& pTextures)
    :   primitiveType(primitiveType)
    ,   indexType(indexType)
    ,   indexCount(indexCount)
    ,   indexBuffer(indexBuffer)
    ,   pTextures(pTextures)
    {
        for (auto&& pTex : this->pTextures)
        {
            pTex->retain();
        }
    }
    
    SubMesh::SubMesh(MTL::PrimitiveType primitiveType,
            MTL::IndexType indexType,
            NS::UInteger indexCount,
            const MeshBuffer& indexBuffer)
    :   primitiveType(primitiveType)
    ,   indexType(indexType)
    ,   indexCount(indexCount)
    ,   indexBuffer(indexBuffer)
    {
        for ( size_t i = 0; i < 3; ++i )
        {
            this->pTextures[i] = nullptr;
        }
    }
    
    SubMesh::SubMesh(const SubMesh& rhs)
    {
        primitiveType = rhs.primitiveType;
        indexType = rhs.indexType;
        indexCount = rhs.indexCount;
        assert(false);
        // TODO: Fix this
        // indexBuffer = rhs.indexBuffer;
        pTextures = rhs.pTextures;
        
        for ( size_t i = 0; i < 3; ++i )
        {
            pTextures[i]->retain();
        }
    }
    
    SubMesh::SubMesh(SubMesh&& rhs)
    {
        primitiveType = rhs.primitiveType;
        indexType = rhs.indexType;
        indexCount = rhs.indexCount;
        indexBuffer = rhs.indexBuffer;
        pTextures = rhs.pTextures;
        
        for ( size_t i = 0; i < 3; ++i )
        {
            pTextures[i]->retain();
            rhs.pTextures[i]->release();
            rhs.pTextures[i] = nullptr;
        }
    }
    
    SubMesh& SubMesh::operator=(const SubMesh& rhs)
    {
        primitiveType = rhs.primitiveType;
        indexType = rhs.indexType;
        indexCount = rhs.indexCount;
        assert(false);
        // TODO: Fix this
        // indexBuffer = rhs.indexBuffer;
        pTextures = rhs.pTextures;
        
        for ( size_t i = 0; i < 3; ++i )
        {
            pTextures[i]->retain();
        }
        
        return *this;
    }
    
    SubMesh& SubMesh::operator=(SubMesh&& rhs)
    {
        primitiveType = rhs.primitiveType;
        indexType = rhs.indexType;
        indexCount = rhs.indexCount;
        indexBuffer = rhs.indexBuffer;
        pTextures = rhs.pTextures;
        
        for ( size_t i = 0; i < 3; ++i )
        {
            pTextures[i]->retain();
            rhs.pTextures[i]->release();
            rhs.pTextures[i] = nullptr;
        }
        
        return *this;
    }
    
    SubMesh::~SubMesh()
    {
        for ( auto&& pTex : pTextures )
        {
            pTex->release();
        }
    }
    
    MTL::PrimitiveType SubMesh::GetPrimitiveType() const
    {
        return primitiveType;
    }
    
    MTL::IndexType SubMesh::GetIndexType() const
    {
        return indexType;
    }
    
    NS::UInteger SubMesh::GetIndexCount() const
    {
        return indexCount;
    }
    
    const MeshBuffer& SubMesh::GetMeshBuffer() const
    {
        return indexBuffer;
    }
    
    const SubmeshTextureArray& SubMesh::GetTextures() const
    {
        return pTextures;
    }
}
