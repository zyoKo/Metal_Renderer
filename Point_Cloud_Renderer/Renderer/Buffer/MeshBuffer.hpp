//
//  MeshBuffer.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#ifndef MeshBuffer_hpp
#define MeshBuffer_hpp

#include <Foundation/Foundation.hpp>

#include "Core/Core.hpp"

FD_MTL

namespace PCR
{
    class MeshBuffer
    {
    public:

        MeshBuffer();
        
        MeshBuffer( MTL::Buffer* pBuffer,
                    NS::UInteger offset,
                    NS::UInteger length,
                    NS::UInteger argumentIndex = NS::UIntegerMax );

        MeshBuffer( const MeshBuffer& rhs );
        
        MeshBuffer& operator=( const MeshBuffer& rhs );

        MeshBuffer( MeshBuffer&& rhs );
        
        MeshBuffer& operator=( MeshBuffer&& rhs );

        ~MeshBuffer();

        MTL::Buffer* getBuffer() const;
        
        NS::UInteger getLength() const;
        
        NS::UInteger getArgumentIndex() const;
        
        NS::UInteger getOffset() const;

        static std::vector< MeshBuffer >
        makeVertexBuffers( MTL::Device* pDevice,
                           const MTL::VertexDescriptor* pDescriptor,
                           NS::UInteger vertexCount,
                           NS::UInteger indexBufferSize );

    private:
        MTL::Buffer* m_pBuffer;
        
        NS::UInteger m_length;
        
        NS::UInteger m_offset;
        
        NS::UInteger m_argumentIndex;
    };
}

#endif /* MeshBuffer_hpp */
