//
//  Mesh.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#ifndef Mesh_hpp
#define Mesh_hpp

#include "Renderer/Buffer/MeshBuffer.hpp"
#include "Renderer/Mesh/SubMesh.hpp"

namespace PCR
{
    class Mesh
    {
    public:
        Mesh() = default;
        
        explicit Mesh(const std::vector<SubMesh>& subMeshes,
                      const std::vector<MeshBuffer>& meshBuffers);
        
        explicit Mesh(const SubMesh& subMesh,
                      const std::vector<MeshBuffer>& meshBuffers);
        
        explicit Mesh( const Mesh& rhs );
        
        explicit Mesh( Mesh&& rhs ) noexcept;
        
        Mesh& operator=( const Mesh& rhs );
        
        Mesh& operator=( Mesh&& rhs ) noexcept;
        
        ~Mesh() = default;
        
        const std::vector<SubMesh>& GetSubMeshes() const;
        
        const std::vector<MeshBuffer>& GetMeshBuffers() const;
        
    private:
        std::vector<SubMesh> subMeshes;
        
        std::vector<MeshBuffer> meshBuffers;
    };
}

#endif /* Mesh_hpp */
