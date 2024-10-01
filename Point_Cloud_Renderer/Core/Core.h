//
//  Core.h
//  Point_Cloud_Renderer
//
//  Created by Vatsalya Yadav on 8/29/24.
//

#pragma once

#define CreateUTF8String(literal) NS::String::string( literal, NS::StringEncoding::UTF8StringEncoding )

#include <string>
#include <Foundation/Foundation.hpp>

namespace PCR
{
    static NS::String* ConvertToNSString( const std::string& stdString )
    {
        NS::String* nsString = (NS::String*)(stdString.c_str());
        return nsString;
    }
}
