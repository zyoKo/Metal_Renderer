#include "MeshFactory.hpp"

#include <cassert>
#include <cmath>
#include <cstring>

#include "Renderer/Buffer/MeshBuffer.hpp"
#include "Renderer/Mesh/SubMesh.hpp"
#include "Renderer/Structures/ShaderTypes.hpp"

namespace PCR::MeshFactory
{
    namespace
    {
        constexpr NS::UInteger POSITION_STRIDE{ sizeof( float ) * 3 };
        constexpr NS::UInteger GENERICS_STRIDE{ sizeof( float ) * 3 + sizeof( float ) * 2 };
        constexpr NS::UInteger GENERICS_TEXCOORD_OFFSET{ sizeof( float ) * 3 };
    }
    
    NS::SharedPtr< MTL::VertexDescriptor > makeVertexDescriptor()
    {
        auto pDesc = NS::TransferPtr( MTL::VertexDescriptor::alloc()->init() );
        
        auto* pPosition = pDesc->attributes()->object( VertexAttributePosition );
        pPosition->setFormat( MTL::VertexFormatFloat3 );
        pPosition->setOffset( 0 );
        pPosition->setBufferIndex( BufferIndexMeshPosition );
        
        auto* pNormal = pDesc->attributes()->object( VertexAttributeNormal );
        pNormal->setFormat( MTL::VertexFormatFloat3 );
        pNormal->setOffset( 0 );
        pNormal->setBufferIndex( BufferIndexMeshGenerics );
        
        auto* pTexCoord = pDesc->attributes()->object( VertexAttributeTexCoord );
        pTexCoord->setFormat( MTL::VertexFormatFloat2 );
        pTexCoord->setOffset( GENERICS_TEXCOORD_OFFSET );
        pTexCoord->setBufferIndex( BufferIndexMeshGenerics );
        
        auto* pPositionLayout = pDesc->layouts()->object( BufferIndexMeshPosition );
        pPositionLayout->setStride( POSITION_STRIDE );
        pPositionLayout->setStepRate( 1 );
        pPositionLayout->setStepFunction( MTL::VertexStepFunctionPerVertex );
        
        auto* pGenericsLayout = pDesc->layouts()->object( BufferIndexMeshGenerics );
        pGenericsLayout->setStride( GENERICS_STRIDE );
        pGenericsLayout->setStepRate( 1 );
        pGenericsLayout->setStepFunction( MTL::VertexStepFunctionPerVertex );
        
        return pDesc;
    }
    
    Mesh makeMesh( MTL::Device* pDevice,
                   const std::vector< VertexData >& vertices,
                   const std::vector< uint16_t >& indices )
    {
        assert( pDevice );
        assert( !vertices.empty() && !indices.empty() );
        assert( vertices.size() <= UINT16_MAX );
        
        auto pDesc = makeVertexDescriptor();
        
        const NS::UInteger indexDataSize = indices.size() * sizeof( uint16_t );
        
        std::vector< MeshBuffer > vertexBuffers = MeshBuffer::makeVertexBuffers( pDevice,
                                                                                 pDesc.get(),
                                                                                 vertices.size(),
                                                                                 indexDataSize );
        assert( !vertexBuffers.empty() );
        
        MTL::Buffer* pBuffer = vertexBuffers.front().getBuffer();
        auto* pBase = static_cast< uint8_t* >( pBuffer->contents() );
        
        // makeVertexBuffers reserves the start of the buffer for the index data.
        std::memcpy( pBase, indices.data(), indexDataSize );
        
        for ( const MeshBuffer& vertexBuffer : vertexBuffers )
        {
            uint8_t* pDst = pBase + vertexBuffer.getOffset();
            
            switch ( vertexBuffer.getArgumentIndex() )
            {
                case BufferIndexMeshPosition:
                    for ( size_t i = 0; i < vertices.size(); ++i )
                    {
                        // simd::float3 is 16 bytes; the GPU layout is a packed 12.
                        std::memcpy( pDst + i * POSITION_STRIDE, &vertices[ i ].position, POSITION_STRIDE );
                    }
                    break;
                    
                case BufferIndexMeshGenerics:
                    for ( size_t i = 0; i < vertices.size(); ++i )
                    {
                        uint8_t* pVertex = pDst + i * GENERICS_STRIDE;
                        std::memcpy( pVertex, &vertices[ i ].normal, sizeof( float ) * 3 );
                        std::memcpy( pVertex + GENERICS_TEXCOORD_OFFSET, &vertices[ i ].texCoord, sizeof( float ) * 2 );
                    }
                    break;
                    
                default:
                    assert( false && "Vertex descriptor uses a buffer index MeshFactory doesn't fill" );
                    break;
            }
        }
        
        MeshBuffer indexBuffer( pBuffer, /* offset */ 0, indexDataSize );
        
        SubMesh subMesh( MTL::PrimitiveTypeTriangle,
                         MTL::IndexTypeUInt16,
                         indices.size(),
                         indexBuffer );
        
        return Mesh( subMesh, vertexBuffers );
    }
    
    Mesh makeCube( MTL::Device* pDevice, float size /* = 1.0f */ )
    {
        const float s = size * 0.5f;
        
        const std::vector< VertexData > vertices =
        {
            //   Positions           Normals             TexCoords
            { { -s, -s, +s }, {  0.f,  0.f,  1.f }, { 0.f, 1.f } },
            { { +s, -s, +s }, {  0.f,  0.f,  1.f }, { 1.f, 1.f } },
            { { +s, +s, +s }, {  0.f,  0.f,  1.f }, { 1.f, 0.f } },
            { { -s, +s, +s }, {  0.f,  0.f,  1.f }, { 0.f, 0.f } },
            
            { { +s, -s, +s }, {  1.f,  0.f,  0.f }, { 0.f, 1.f } },
            { { +s, -s, -s }, {  1.f,  0.f,  0.f }, { 1.f, 1.f } },
            { { +s, +s, -s }, {  1.f,  0.f,  0.f }, { 1.f, 0.f } },
            { { +s, +s, +s }, {  1.f,  0.f,  0.f }, { 0.f, 0.f } },
            
            { { +s, -s, -s }, {  0.f,  0.f, -1.f }, { 0.f, 1.f } },
            { { -s, -s, -s }, {  0.f,  0.f, -1.f }, { 1.f, 1.f } },
            { { -s, +s, -s }, {  0.f,  0.f, -1.f }, { 1.f, 0.f } },
            { { +s, +s, -s }, {  0.f,  0.f, -1.f }, { 0.f, 0.f } },
            
            { { -s, -s, -s }, { -1.f,  0.f,  0.f }, { 0.f, 1.f } },
            { { -s, -s, +s }, { -1.f,  0.f,  0.f }, { 1.f, 1.f } },
            { { -s, +s, +s }, { -1.f,  0.f,  0.f }, { 1.f, 0.f } },
            { { -s, +s, -s }, { -1.f,  0.f,  0.f }, { 0.f, 0.f } },
            
            { { -s, +s, +s }, {  0.f,  1.f,  0.f }, { 0.f, 1.f } },
            { { +s, +s, +s }, {  0.f,  1.f,  0.f }, { 1.f, 1.f } },
            { { +s, +s, -s }, {  0.f,  1.f,  0.f }, { 1.f, 0.f } },
            { { -s, +s, -s }, {  0.f,  1.f,  0.f }, { 0.f, 0.f } },
            
            { { -s, -s, -s }, {  0.f, -1.f,  0.f }, { 0.f, 1.f } },
            { { +s, -s, -s }, {  0.f, -1.f,  0.f }, { 1.f, 1.f } },
            { { +s, -s, +s }, {  0.f, -1.f,  0.f }, { 1.f, 0.f } },
            { { -s, -s, +s }, {  0.f, -1.f,  0.f }, { 0.f, 0.f } }
        };
        
        const std::vector< uint16_t > indices =
        {
             0,  1,  2,  2,  3,  0, /* front  */
             4,  5,  6,  6,  7,  4, /* right  */
             8,  9, 10, 10, 11,  8, /* back   */
            12, 13, 14, 14, 15, 12, /* left   */
            16, 17, 18, 18, 19, 16, /* top    */
            20, 21, 22, 22, 23, 20, /* bottom */
        };
        
        return makeMesh( pDevice, vertices, indices );
    }
    
    Mesh makePyramid( MTL::Device* pDevice, float size /* = 1.0f */ )
    {
        const float s = size * 0.5f;
        const simd::float3 apex = { 0.f, +s, 0.f };
        
        const simd::float3 corners[ 4 ] =
        {
            { -s, -s, +s }, { +s, -s, +s }, { +s, -s, -s }, { -s, -s, -s }
        };
        
        std::vector< VertexData > vertices;
        std::vector< uint16_t > indices;
        
        for ( int side = 0; side < 4; ++side )
        {
            const simd::float3 a = corners[ side ];
            const simd::float3 b = corners[ ( side + 1 ) % 4 ];
            const simd::float3 normal = simd::normalize( simd::cross( b - a, apex - a ) );
            
            const auto base = static_cast< uint16_t >( vertices.size() );
            vertices.push_back( { a,    normal, { 0.0f, 1.f } } );
            vertices.push_back( { b,    normal, { 1.0f, 1.f } } );
            vertices.push_back( { apex, normal, { 0.5f, 0.f } } );
            indices.insert( indices.end(), { base, static_cast< uint16_t >( base + 1 ), static_cast< uint16_t >( base + 2 ) } );
        }
        
        const simd::float3 down = { 0.f, -1.f, 0.f };
        const auto base = static_cast< uint16_t >( vertices.size() );
        vertices.push_back( { corners[ 3 ], down, { 0.f, 1.f } } );
        vertices.push_back( { corners[ 2 ], down, { 1.f, 1.f } } );
        vertices.push_back( { corners[ 1 ], down, { 1.f, 0.f } } );
        vertices.push_back( { corners[ 0 ], down, { 0.f, 0.f } } );
        indices.insert( indices.end(), {
            base, static_cast< uint16_t >( base + 1 ), static_cast< uint16_t >( base + 2 ),
            static_cast< uint16_t >( base + 2 ), static_cast< uint16_t >( base + 3 ), base } );
        
        return makeMesh( pDevice, vertices, indices );
    }
    
    Mesh makeSphere( MTL::Device* pDevice,
                     float radius /* = 0.5f */,
                     uint16_t slices /* = 32 */,
                     uint16_t stacks /* = 16 */ )
    {
        assert( slices >= 3 && stacks >= 2 );
        assert( static_cast< size_t >( slices + 1 ) * ( stacks + 1 ) <= UINT16_MAX );
        
        const uint16_t ringSize = slices + 1; // seam column is duplicated so UVs wrap
        
        std::vector< VertexData > vertices;
        vertices.reserve( static_cast< size_t >( ringSize ) * ( stacks + 1 ) );
        
        for ( uint16_t stack = 0; stack <= stacks; ++stack )
        {
            const float v = static_cast< float >( stack ) / stacks;
            const float phi = v * static_cast< float >( M_PI );
            
            for ( uint16_t slice = 0; slice <= slices; ++slice )
            {
                const float u = static_cast< float >( slice ) / slices;
                const float theta = u * 2.0f * static_cast< float >( M_PI );
                
                const simd::float3 normal = { sinf( phi ) * cosf( theta ),
                                              cosf( phi ),
                                              sinf( phi ) * sinf( theta ) };
                
                vertices.push_back( { normal * radius, normal, { u, v } } );
            }
        }
        
        std::vector< uint16_t > indices;
        indices.reserve( static_cast< size_t >( slices ) * stacks * 6 );
        
        for ( uint16_t stack = 0; stack < stacks; ++stack )
        {
            for ( uint16_t slice = 0; slice < slices; ++slice )
            {
                const uint16_t a = stack * ringSize + slice;
                const uint16_t b = a + ringSize;
                
                // Counter-clockwise from outside, to match back-face culling.
                indices.insert( indices.end(), { a, static_cast< uint16_t >( a + 1 ), b } );
                indices.insert( indices.end(), { static_cast< uint16_t >( a + 1 ), static_cast< uint16_t >( b + 1 ), b } );
            }
        }
        
        return makeMesh( pDevice, vertices, indices );
    }
}
