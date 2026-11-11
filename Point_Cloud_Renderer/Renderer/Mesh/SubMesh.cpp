//
//  SubMesh.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#include "SubMesh.hpp"

#include <utility>

#include "Renderer/Buffer/MeshBuffer.hpp"

namespace PCR
{
    SubMesh::SubMesh()
    :   primitiveType{ MTL::PrimitiveTypeTriangle }
    ,   indexType{ MTL::IndexTypeUInt16 }
    ,   indexCount{ 0 }
    ,   indexBuffer{}
    ,   pTextures{}
    { }
    
    SubMesh::SubMesh( MTL::PrimitiveType primitiveType,
                      MTL::IndexType indexType,
                      NS::UInteger indexCount,
                      const MeshBuffer& indexBuffer,
                      const SubmeshTextureArray& pTextures )
    :   primitiveType{ primitiveType }
    ,   indexType{ indexType }
    ,   indexCount{ indexCount }
    ,   indexBuffer{ indexBuffer }
    ,   pTextures{ pTextures }
    {
        retainTextures();
    }
    
    SubMesh::SubMesh( MTL::PrimitiveType primitiveType,
                      MTL::IndexType indexType,
                      NS::UInteger indexCount,
                      const MeshBuffer& indexBuffer )
    :   primitiveType{ primitiveType }
    ,   indexType{ indexType }
    ,   indexCount{ indexCount }
    ,   indexBuffer{ indexBuffer }
    ,   pTextures{}
    { }
    
    SubMesh::SubMesh( const SubMesh& rhs )
    :   primitiveType{ rhs.primitiveType }
    ,   indexType{ rhs.indexType }
    ,   indexCount{ rhs.indexCount }
    ,   indexBuffer{ rhs.indexBuffer }
    ,   pTextures{ rhs.pTextures }
    {
        retainTextures();
    }
    
    SubMesh::SubMesh( SubMesh&& rhs ) noexcept
    :   primitiveType{ rhs.primitiveType }
    ,   indexType{ rhs.indexType }
    ,   indexCount{ rhs.indexCount }
    ,   indexBuffer{ std::move( rhs.indexBuffer ) }
    ,   pTextures{ rhs.pTextures }
    {
        rhs.pTextures.fill( nullptr );
    }
    
    SubMesh& SubMesh::operator=( const SubMesh& rhs )
    {
        if ( this != &rhs )
        {
            SubmeshTextureArray oldTextures = pTextures;
            
            primitiveType = rhs.primitiveType;
            indexType = rhs.indexType;
            indexCount = rhs.indexCount;
            indexBuffer = rhs.indexBuffer;
            pTextures = rhs.pTextures;
            retainTextures();
            
            for ( MTL::Texture* pTex : oldTextures )
            {
                if ( pTex )
                {
                    pTex->release();
                }
            }
        }
        
        return *this;
    }
    
    SubMesh& SubMesh::operator=( SubMesh&& rhs ) noexcept
    {
        if ( this != &rhs )
        {
            releaseTextures();
            
            primitiveType = rhs.primitiveType;
            indexType = rhs.indexType;
            indexCount = rhs.indexCount;
            indexBuffer = std::move( rhs.indexBuffer );
            pTextures = rhs.pTextures;
            rhs.pTextures.fill( nullptr );
        }
        
        return *this;
    }
    
    SubMesh::~SubMesh()
    {
        releaseTextures();
    }
    
    void SubMesh::retainTextures()
    {
        for ( MTL::Texture* pTex : pTextures )
        {
            if ( pTex )
            {
                pTex->retain();
            }
        }
    }
    
    void SubMesh::releaseTextures()
    {
        for ( MTL::Texture*& pTex : pTextures )
        {
            if ( pTex )
            {
                pTex->release();
                pTex = nullptr;
            }
        }
    }
    
    MTL::PrimitiveType SubMesh::getPrimitiveType() const
    {
        return primitiveType;
    }
    
    MTL::IndexType SubMesh::getIndexType() const
    {
        return indexType;
    }
    
    NS::UInteger SubMesh::getIndexCount() const
    {
        return indexCount;
    }
    
    const MeshBuffer& SubMesh::getMeshBuffer() const
    {
        return indexBuffer;
    }
    
    const SubmeshTextureArray& SubMesh::getTextures() const
    {
        return pTextures;
    }
}
