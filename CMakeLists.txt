cmake_minimum_required(VERSION 3.13.0)
project(DX12Lib VERSION 0.1.0)

include(CTest)
enable_testing()

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
set(ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/thirdParty)

# include directories
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${THIRD_PARTY_DIR}/DirectX-Headers/include)
include_directories(${THIRD_PARTY_DIR}/DirectXTex/DirectXTex)
include_directories(${THIRD_PARTY_DIR}/assimp/include)
include_directories(${CMAKE_BINARY_DIR}/thirdParty/assimp/include)
include_directories(${THIRD_PARTY_DIR}/DirectXShaderCompiler/inc)
include_directories(${THIRD_PARTY_DIR}/rapidjson/include)
include_directories(${THIRD_PARTY_DIR}/winpixeventruntime/Include)

# message(STATUS "foo include dir: ${THIRD_PARTY_DIR}/assimp/include")
# include_directories("C:\\code\\DX12Lib\\thirdParty\\assimp\\include")

# 

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Build as submodule" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/assimp) #only needed first time to get bins
add_subdirectory(${THIRD_PARTY_DIR}/DirectX-Headers)
set(BUILD_TOOLS OFF CACHE BOOL "Build tex command-line tools" FORCE)
set(BUILD_SAMPLE OFF CACHE BOOL "Build DDSView sample" FORCE)
set(BUILD_DX11 OFF CACHE BOOL "Build with DirectX11 Runtime support" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/DirectXTex)


# source files
add_executable(DX12Lib WIN32 main.cpp
    Application.cpp
    DXApp.cpp
    DXAppImplementation.cpp
    FileManager.cpp
    RenderModel.cpp
    ResourceManager.cpp
    ShaderManager.cpp
    Transformations.cpp
    Level.cpp
    FreeCamera.cpp
    HeapBuffer.cpp
    ResourceDescriptor.cpp
    GpuResource.cpp
    DescriptorHeapCollection.cpp
    LevelEntity.cpp
    ConstantBufferManager.cpp
    CommandQueue.cpp
    GfxCommandQueue.cpp
    Techniques.cpp
    RenderQuad.cpp
    RenderObject.cpp
    GpuDataManager.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_options(${PROJECT_NAME} PRIVATE "/entry:WinMainCRTStartup")
target_link_libraries(${PROJECT_NAME} d3d12.lib)
target_link_libraries(${PROJECT_NAME} dxgi.lib)
# target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/thirdParty/assimp/lib/$<CONFIGURATION>/assimp-vc143-mtd.lib)
target_link_libraries(${PROJECT_NAME} assimp)
target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_DIR}/DirectXShaderCompiler/lib/x64/dxcompiler.lib)
target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_DIR}/winpixeventruntime/bin/x64/WinPixEventRuntime.lib)
target_link_libraries(${PROJECT_NAME} DirectX-Guids)
target_link_libraries(${PROJECT_NAME} DirectXTex)

# Copy thirdparty bins
set(ASSIMP_BIN_DIR ${CMAKE_BINARY_DIR}/thirdParty/assimp/bin/)
set(DXC_BIN_DIR ${THIRD_PARTY_DIR}/DirectXShaderCompiler/bin/x64)
set(WPER_BIN_DIR ${THIRD_PARTY_DIR}/winpixeventruntime/bin/x64)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSIMP_BIN_DIR}/assimp-vc143-mtd.dll"
    "${CMAKE_BINARY_DIR}"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSIMP_BIN_DIR}/assimp-vc143-mtd.pdb"
    "${CMAKE_BINARY_DIR}"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DXC_BIN_DIR}/dxcompiler.dll"
    "${CMAKE_BINARY_DIR}"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DXC_BIN_DIR}/dxil.dll"
    "${CMAKE_BINARY_DIR}"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${WPER_BIN_DIR}/WinPixEventRuntime.dll"
    "${CMAKE_BINARY_DIR}"
)