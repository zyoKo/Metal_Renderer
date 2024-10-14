//
//  Core.h
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#ifndef Core_hpp
#define Core_hpp

#define CreateUTF8String(literal) NS::String::string( literal, NS::StringEncoding::UTF8StringEncoding )

#define FD_MTK                      \
namespace MTK                       \
{                                   \
    class View;                     \
}

#define FD_NS                       \
namespace NS {                      \
    class String;                   \
    class Window;                   \
}

#define FD_MTL                      \
namespace MTL {                     \
    class Device;                   \
    class CommandQueue;             \
    class RenderPipelineState;      \
    class Library;                  \
    class Buffer;                   \
    class DepthStencilState;        \
    class Texture;                  \
    class ComputePipelineState;     \
    class VertexDescriptor;         \
}

#endif  /* Core_hpp */
