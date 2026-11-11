//
//  ShaderTypes.hpp
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 10/14/24.
//

#ifndef ShaderTypes_hpp
#define ShaderTypes_hpp

enum VertexAttribute
{
    VertexAttributePosition  = 0,
    VertexAttributeTexCoord  = 1,
    VertexAttributeNormal    = 2,
    VertexAttributeTangent   = 3,
    VertexAttributeBiTangent = 4,
};

enum TextureIndex
{
    TextureIndexAlbedo   = 0,
    TextureIndexSpecular = 1,
    TextureIndexNormal   = 2,
    TextureIndexShadow   = 3,
    TextureIndexAlpha    = 4
};

enum BufferIndex
{
    BufferIndexMeshPosition = 0,
    BufferIndexMeshGenerics = 1,
    BufferIndexInstanceData = 2,
    BufferIndexCameraData   = 3
};

#endif /* ShaderTypes_hpp */
