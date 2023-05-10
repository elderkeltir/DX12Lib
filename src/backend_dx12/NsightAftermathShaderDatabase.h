//*********************************************************
//
// Copyright (c) 2019-2022, NVIDIA CORPORATION. All rights reserved.
// 
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
// 
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
// 
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.
//
//*********************************************************

#pragma once

#include <map>
#include <mutex>
#include <vector>

#include "NsightAftermathHelpers.h"

//*********************************************************
// Implements a very simple shader database to help demonstrate
// how to use the Nsight Aftermath GPU crash dump decoder API.
//
// In a real world scenario this would be part of an offline
// analysis tool. This is for demonstration purposes only!
//
class ShaderDatabase
{
public:
    ShaderDatabase(const std::string& shader_dir);
    ~ShaderDatabase();

    // Find a shader bytecode binary by shader hash.
    bool FindShaderBinary(const GFSDK_Aftermath_ShaderBinaryHash& shaderHash, std::vector<uint8_t>& shader) const;

    // Find a source shader debug info by shader debug name generated by the DXC compiler.
    bool FindSourceShaderDebugData(const GFSDK_Aftermath_ShaderDebugName& shaderDebugName, std::vector<uint8_t>& debugData) const;

private:

    void AddShaderBinary(const char* filePath);
    void AddSourceShaderDebugData(const char* filePath, const char* fileName);

    static bool ReadFile(const char* filename, std::vector<uint8_t>& data);

    // List of shader binaries by ShaderBinaryHash.
    std::map<GFSDK_Aftermath_ShaderBinaryHash, std::vector<uint8_t>> m_shaderBinaries;

    // List of available source shader debug information.
    std::map<GFSDK_Aftermath_ShaderDebugName, std::vector<uint8_t>> m_sourceShaderDebugData;
};
