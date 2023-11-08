cmake_minimum_required(VERSION 3.13.0)
project(backend_dx12 VERSION 0.1.0)

# dllexport
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# definitions
ADD_DEFINITIONS(-D_AFXDLL)
ADD_DEFINITIONS(-DWIN32)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_DEBUG)
ADD_DEFINITIONS(-D_WINDOWS)
# ADD_DEFINITIONS(-DUSE_PIX_DEBUG) # for attaching only

# VARs
set(CMAKE_BUILD_PARALLEL_LEVEL 16)
set(CMAKE_MFC_FLAG 2)
set(CMAKE_CXX_STANDARD 17) # no need to manually adjust the CXXFLAGS
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(ROOT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/../../thirdParty)
set(THIRD_PARTY_IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)

# source files
add_library(${PROJECT_NAME} SHARED
    # imgui
    ${THIRD_PARTY_IMGUI_DIR}/imgui.cpp
    ${THIRD_PARTY_IMGUI_DIR}/imgui_draw.cpp
    ${THIRD_PARTY_IMGUI_DIR}/imgui_tables.cpp
    ${THIRD_PARTY_IMGUI_DIR}/imgui_widgets.cpp
    ${THIRD_PARTY_IMGUI_DIR}/backends/imgui_impl_dx12.cpp
    ${THIRD_PARTY_IMGUI_DIR}/backends/imgui_impl_win32.cpp
    #
    "ConsoleCommands.cpp"
    "Console.cpp"
    "ShaderManager.cpp"
    "HeapBuffer.cpp"
    "ResourceDescriptor.cpp"
    "DxGpuResource.cpp"
    "DescriptorHeapCollection.cpp"
    "CommandQueue.cpp"
    "Techniques.cpp"
    "DynamicGpuHeap.cpp"
    "ImguiHelper.cpp"
    "CommandList.cpp"
    "Fence.cpp"
    "SwapChain.cpp"
    "DxBackend.cpp"
    "DxDevice.cpp"
    "RootSignature.cpp"
    "TextureLoader.cpp")
    
# include directories
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}")
target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_SOURCE_DIR}/../backend_interface")
target_include_directories(${PROJECT_NAME} PRIVATE "${THIRD_PARTY_DIR}/DirectX-Headers/include")
target_include_directories(${PROJECT_NAME} PRIVATE "${THIRD_PARTY_DIR}/DirectXShaderCompiler/inc")
target_include_directories(${PROJECT_NAME} PRIVATE "${THIRD_PARTY_DIR}/winpixeventruntime/Include")
target_include_directories(${PROJECT_NAME} PRIVATE "${THIRD_PARTY_DIR}/imgui")
target_include_directories(${PROJECT_NAME} PRIVATE "${THIRD_PARTY_DIR}/DirectXTex/DirectXTex")

# link libs
target_link_libraries(${PROJECT_NAME} d3d12.lib)
target_link_libraries(${PROJECT_NAME} dxgi.lib)
target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_DIR}/DirectXShaderCompiler/lib/x64/dxcompiler.lib)
target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_DIR}/winpixeventruntime/bin/x64/WinPixEventRuntime.lib)
target_link_libraries(${PROJECT_NAME} DirectX-Guids)
target_link_libraries(${PROJECT_NAME} DirectXTex)

# Copy thirdparty bins
set(DXC_BIN_DIR ${THIRD_PARTY_DIR}/DirectXShaderCompiler/bin/x64)
set(WPER_BIN_DIR ${THIRD_PARTY_DIR}/winpixeventruntime/bin/x64)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DXC_BIN_DIR}/dxcompiler.dll"
    "${CMAKE_BINARY_DIR}/src"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DXC_BIN_DIR}/dxil.dll"
    "${CMAKE_BINARY_DIR}/src"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${WPER_BIN_DIR}/WinPixEventRuntime.dll"
    "${CMAKE_BINARY_DIR}/src"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/src/backend_dx12/backend_dx12d.dll"
    "${CMAKE_BINARY_DIR}/src"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/src/backend_dx12/backend_dx12d.pdb"
    "${CMAKE_BINARY_DIR}/src"
)