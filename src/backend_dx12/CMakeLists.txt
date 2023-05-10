cmake_minimum_required(VERSION 3.13.0)
project(backend_dx12 VERSION 0.1.0)

include(CTest)
enable_testing()


set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

ADD_DEFINITIONS(-D_AFXDLL)
ADD_DEFINITIONS(-DWIN32)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_DEBUG)
ADD_DEFINITIONS(-D_WINDOWS)
# ADD_DEFINITIONS(-DUSE_PIX_DEBUG) # for attaching only
set(USE_NV_AM 1)


# VARs
set(CMAKE_BUILD_PARALLEL_LEVEL 16)
set(CMAKE_MFC_FLAG 2)
set(CMAKE_CXX_STANDARD 17) # no need to manually adjust the CXXFLAGS
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/../../thirdParty)
set(THIRD_PARTY_IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)

# include directories
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../backend_interface)
include_directories(${THIRD_PARTY_DIR}/DirectX-Headers/include)
include_directories(${THIRD_PARTY_DIR}/DirectXShaderCompiler/inc)
include_directories(${THIRD_PARTY_DIR}/winpixeventruntime/Include)
include_directories(${THIRD_PARTY_DIR}/imgui)
include_directories(${THIRD_PARTY_DIR}/DirectXTex/DirectXTex)

if(DEFINED USE_NV_AM)
ADD_DEFINITIONS(-DUSE_NSIGHT_AFTERMATH) # for crash handling
set(NVIDIA_AFTERMATH_DIR $ENV{NSIGHT_AFTERMATH_SDK})
if(NVIDIA_AFTERMATH_DIR STREQUAL "")
message(FATAL_ERROR "NSIGHT_AFTERMATH_SDK environment variable isn't defined in the system, CMake will exit." )
endif()
include_directories(${NVIDIA_AFTERMATH_DIR}/include)
endif() 

# message(STATUS "foo include dir: ${THIRD_PARTY_DIR}/assimp/include")
# include_directories("C:\\code\\DX12Lib\\thirdParty\\assimp\\include")

# 


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
    "GpuResource.cpp"
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
    "TextureLoader.cpp"
    "NsightAftermathShaderDatabase.cpp"
    "NsightAftermathGpuCrashTracker.cpp"
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_libraries(${PROJECT_NAME} d3d12.lib)
target_link_libraries(${PROJECT_NAME} dxgi.lib)
target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_DIR}/DirectXShaderCompiler/lib/x64/dxcompiler.lib)
target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_DIR}/winpixeventruntime/bin/x64/WinPixEventRuntime.lib)
target_link_libraries(${PROJECT_NAME} DirectX-Guids)
target_link_libraries(${PROJECT_NAME} DirectXTex)

if(DEFINED USE_NV_AM)
target_link_libraries(${PROJECT_NAME} ${NVIDIA_AFTERMATH_DIR}/lib/x64/GFSDK_Aftermath_Lib.x64.lib)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${NVIDIA_AFTERMATH_DIR}/lib/x64/GFSDK_Aftermath_Lib.x64.dll"
    "${CMAKE_BINARY_DIR}/src"
)
endif()

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
)