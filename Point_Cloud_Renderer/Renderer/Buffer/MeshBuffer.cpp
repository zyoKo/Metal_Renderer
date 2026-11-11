//
//  MeshBuffer.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#include "MeshBuffer.hpp"

#include <set>
#include <utility>

#include <MetalKit/MetalKit.hpp>

#include "Math/Utility.hpp"
 
namespace PCR
{
    MeshBuffer::MeshBuffer()
    :   m_pBuffer{ nullptr }
    ,   m_length{ 0 }
    ,   m_offset{ 0 }
    ,   m_argumentIndex{ 0 }
    { }
    
    MeshBuffer::MeshBuffer( MTL::Buffer* pBuffer,
                            NS::UInteger offset,
                            NS::UInteger length,
                            NS::UInteger argumentIndex /* = NS::UIntegerMax */ )
    :   m_pBuffer{ pBuffer ? pBuffer->retain() : nullptr }
    ,   m_length{ length }
    ,   m_offset{ offset }
    ,   m_argumentIndex{ argumentIndex }
    { }
    
    MeshBuffer::MeshBuffer( const MeshBuffer& rhs )
    :   m_pBuffer{ rhs.m_pBuffer ? rhs.m_pBuffer->retain() : nullptr }
    ,   m_length{ rhs.m_length }
    ,   m_offset{ rhs.m_offset }
    ,   m_argumentIndex{ rhs.m_argumentIndex }
    { }
    
    MeshBuffer& MeshBuffer::operator=( const MeshBuffer& rhs )
    {
        if ( this != &rhs )
        {
            // Retain first: both may hold the same MTL::Buffer.
            MTL::Buffer* pNewBuffer = rhs.m_pBuffer ? rhs.m_pBuffer->retain() : nullptr;
            if ( m_pBuffer )
            {
                m_pBuffer->release();
            }
            m_pBuffer = pNewBuffer;
            
            m_length = rhs.m_length;
            m_offset = rhs.m_offset;
            m_argumentIndex = rhs.m_argumentIndex;
        }
        
        return *this;
    }
    
    MeshBuffer::MeshBuffer( MeshBuffer&& rhs ) noexcept
    :   m_pBuffer{ std::exchange( rhs.m_pBuffer, nullptr ) }
    ,   m_length{ rhs.m_length }
    ,   m_offset{ rhs.m_offset }
    ,   m_argumentIndex{ rhs.m_argumentIndex }
    { }
    
    MeshBuffer& MeshBuffer::operator=( MeshBuffer&& rhs ) noexcept
    {
        if ( this != &rhs )
        {
            if ( m_pBuffer )
            {
                m_pBuffer->release();
            }
            m_pBuffer = std::exchange( rhs.m_pBuffer, nullptr );
            
            m_length = rhs.m_length;
            m_offset = rhs.m_offset;
            m_argumentIndex = rhs.m_argumentIndex;
        }
        
        return *this;
    }
    
    MeshBuffer::~MeshBuffer()
    {
        if ( m_pBuffer )
        {
            m_pBuffer->release();
        }
    }
    
    MTL::Buffer* MeshBuffer::getBuffer() const
    {
        return m_pBuffer;
    }
    
    NS::UInteger MeshBuffer::getLength() const
    {
        return m_length;
    }
    
    NS::UInteger MeshBuffer::getArgumentIndex() const
    {
        return m_argumentIndex;
    }
    
    NS::UInteger MeshBuffer::getOffset() const
    {
        return m_offset;
    }
    
    /* [[ nodiscard ]] static */
    std::vector< MeshBuffer >
    MeshBuffer::makeVertexBuffers( MTL::Device* pDevice,
                                   const MTL::VertexDescriptor* pDescriptor,
                                   NS::UInteger vertexCount,
                                   NS::UInteger indexBufferSize )
    {
        std::set< NS::UInteger > bufferIndicesUsed;

        for( int i = 0; i < 31; ++i )
        {
            bufferIndicesUsed.insert( pDescriptor->attributes()->object( i )->bufferIndex() );
        }

        std::vector< MeshBuffer > vertexBuffers;

        indexBufferSize = Math::alignSize( indexBufferSize, 256 );

        NS::UInteger bufferLength = indexBufferSize;
        for( auto bufferIndex : bufferIndicesUsed )
        {
            NS::UInteger offset = bufferLength;
            
            size_t inputSize = vertexCount * pDescriptor->layouts()->object( bufferIndex )->stride();
            NS::UInteger sectionLength = Math::alignSize( inputSize, 256 );

            bufferLength += sectionLength;

            vertexBuffers.emplace_back( MeshBuffer( nullptr, offset, sectionLength, bufferIndex ) );
        }

        MTL::Buffer* pMetalBuffer = pDevice->newBuffer( bufferLength, MTL::ResourceStorageModeShared );

        for( auto&& vertexBuffer : vertexBuffers )
        {
            // safe to assume vertexBuffer.m_pBuffer is nullptr, no need to release it here
            vertexBuffer.m_pBuffer = pMetalBuffer->retain();
        }
        
        pMetalBuffer->release();
        
        return vertexBuffers;
    }
}
