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

# VARs
set(CMAKE_MFC_FLAG 2)
set(CMAKE_CXX_STANDARD 17) # no need to manually adjust the CXXFLAGS
# set(ROOT_FOLDER ${PROJECT_SOURCE_DIR}/..)
set(ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/thirdParty)

# include directories
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${THIRD_PARTY_DIR}/assimp/include)
include_directories(${CMAKE_BINARY_DIR}/thirdParty/assimp/include)
include_directories(${THIRD_PARTY_DIR}/DirectXTK12/Inc)
include_directories(${THIRD_PARTY_DIR}/DirectXShaderCompiler/inc)
include_directories(${THIRD_PARTY_DIR}/rapidjson/include)
# message(STATUS "foo include dir: ${THIRD_PARTY_DIR}/assimp/include")
# include_directories("C:\\code\\DX12Lib\\thirdParty\\assimp\\include")

# 
# add_subdirectory(${THIRD_PARTY_DIR}/assimp)
# add_subdirectory(${THIRD_PARTY_DIR}/DirectXTK12)

# source files
add_executable(DX12Lib WIN32 main.cpp
    Application.cpp
    DXApp.cpp
    DXAppImplementation.cpp
    FileManager.cpp
    Texture.cpp
    RenderMesh.cpp
    TextureManager.cpp
    ResourceManager.cpp
    ShaderManager.cpp
    Transformations.cpp
    Level.cpp
    FreeCamera.cpp
    HeapBuffer.cpp
    ResourceDescriptor.cpp
    GpuResource.cpp
    DescriptorHeapCollection.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

target_link_options(${PROJECT_NAME} PRIVATE "/entry:WinMainCRTStartup")
target_link_libraries(${PROJECT_NAME} d3d12.lib)
target_link_libraries(${PROJECT_NAME} dxgi.lib)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/thirdParty/assimp/lib/$<CONFIGURATION>/assimp-vc143-mtd.lib)
target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/bin/CMake/$<CONFIGURATION>/DirectXTK12d.lib)
target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_DIR}/DirectXShaderCompiler/lib/x64/dxcompiler.lib)

# Copy thirdparty bins
set(ASSIMP_BIN_DIR ${CMAKE_BINARY_DIR}/thirdParty/assimp/bin/$<CONFIGURATION>)
set(DXC_BIN_DIR ${THIRD_PARTY_DIR}/DirectXShaderCompiler/bin/x64)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSIMP_BIN_DIR}/assimp-vc143-mtd.dll"
    "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSIMP_BIN_DIR}/assimp-vc143-mtd.pdb"
    "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DXC_BIN_DIR}/dxcompiler.dll"
    "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${DXC_BIN_DIR}/dxil.dll"
    "${CMAKE_BINARY_DIR}/$<CONFIGURATION>"
)