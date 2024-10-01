//
//  Mesh.cpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 9/29/24.
//

#include "Mesh.hpp"

namespace PCR
{
    Mesh::Mesh(const std::vector<SubMesh>& subMeshes,
               const std::vector<MeshBuffer>& meshBuffers)
    :   subMeshes(subMeshes)
    ,   meshBuffers(meshBuffers)
    { }
    
    Mesh::Mesh(const SubMesh& subMesh,
               const std::vector<MeshBuffer>& meshBuffers)
    :   meshBuffers(meshBuffers)
    {
        this->subMeshes.emplace_back(subMesh);
    }
    
    Mesh::Mesh( const Mesh& rhs )
    :   subMeshes(rhs.subMeshes)
    ,   meshBuffers(rhs.meshBuffers)
    { }
    
    Mesh::Mesh( Mesh&& rhs ) noexcept
    :   subMeshes(rhs.subMeshes)
    ,   meshBuffers(rhs.meshBuffers)
    { }
    
    Mesh& Mesh::operator=( const Mesh& rhs )
    {
        subMeshes = rhs.subMeshes;
        meshBuffers = rhs.meshBuffers;
        
        return *this;
    }
    
    Mesh& Mesh::operator=( Mesh&& rhs ) noexcept
    {
        subMeshes = rhs.subMeshes;
        meshBuffers = rhs.meshBuffers;
        
        return *this;
    }
    
    const std::vector<SubMesh>& Mesh::GetSubMeshes() const
    {
        return subMeshes;
    }
    
    const std::vector<MeshBuffer>& Mesh::GetMeshBuffers() const
    {
        return meshBuffers;
    }
}
