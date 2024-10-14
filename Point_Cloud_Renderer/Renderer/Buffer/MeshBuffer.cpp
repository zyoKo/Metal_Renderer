//
//  MeshBuffer.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#include "MeshBuffer.hpp"

#include <MetalKit/MetalKit.hpp>
 
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
    :   m_pBuffer{ pBuffer->retain() }
    ,   m_length{ length }
    ,   m_offset{ offset }
    ,   m_argumentIndex{ argumentIndex }
    { }

    MeshBuffer::MeshBuffer( const MeshBuffer& rhs )
    :   m_pBuffer{ rhs.m_pBuffer->retain() }
    ,   m_length{ rhs.m_length }
    ,   m_offset{ rhs.m_offset }
    ,   m_argumentIndex{ rhs.m_argumentIndex }
    { }

    MeshBuffer& MeshBuffer::operator=( const MeshBuffer& rhs )
    {
        m_pBuffer = rhs.m_pBuffer->retain();
        m_length = rhs.m_length;
        m_offset = rhs.m_offset;
        m_argumentIndex = rhs.m_argumentIndex;
        
        return *this;
    }

    MeshBuffer::MeshBuffer( MeshBuffer&& rhs )
    :   m_pBuffer{ rhs.m_pBuffer->retain() }
    ,   m_length{ rhs.m_length }
    ,   m_offset{ rhs.m_offset }
    ,   m_argumentIndex{ rhs.m_argumentIndex }
    {
        rhs.m_pBuffer->release();
    }

    MeshBuffer& MeshBuffer::operator=( MeshBuffer&& rhs )
    {
        m_pBuffer = rhs.m_pBuffer->retain();
        rhs.m_pBuffer->release();
        
        m_length = rhs.m_length;
        m_offset = rhs.m_offset;
        m_argumentIndex = rhs.m_argumentIndex;
        
        return *this;
    }

    MeshBuffer::~MeshBuffer()
    {
        m_pBuffer->release();
    }
}
