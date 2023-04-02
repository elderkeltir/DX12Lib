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
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/../thirdParty)


# include directories
include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/backend_interface)


include_directories(${THIRD_PARTY_DIR}/assimp/include)
include_directories(${CMAKE_BINARY_DIR}/thirdParty/assimp/include)
include_directories(${THIRD_PARTY_DIR}/rapidjson/include)

# message(STATUS "foo include dir: ${THIRD_PARTY_DIR}/assimp/include")
# include_directories("C:\\code\\DX12Lib\\thirdParty\\assimp\\include")

# 
add_subdirectory(backend_dx12)

# source files
add_executable(DX12Lib WIN32 main.cpp
    WinApplication.cpp
    FileManager.cpp
    Frontend.cpp
    RenderModel.cpp
    ResourceManager.cpp

    Transformations.cpp
    Level.cpp
    FreeCamera.cpp

    LevelEntity.cpp
    ConstantBufferManager.cpp

    RenderQuad.cpp
    RenderObject.cpp
    GpuDataManager.cpp
    MaterialManager.cpp
    RenderMesh.cpp
    SkyBox.cpp
    SSAO.cpp

    Plane.cpp
    Sun.cpp
    Reflections.cpp
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
#include(CPack)

target_compile_options(${PROJECT_NAME} PUBLIC  "/EHsc")

target_link_options(${PROJECT_NAME} PRIVATE "/entry:WinMainCRTStartup")

# target_link_libraries(${PROJECT_NAME} ${CMAKE_BINARY_DIR}/thirdParty/assimp/lib/$<CONFIGURATION>/assimp-vc143-mtd.lib)
target_link_libraries(${PROJECT_NAME} assimp)

#target_link_libraries(${PROJECT_NAME} DirectX-Guids)


target_link_libraries(${PROJECT_NAME} backend_dx12)


# Copy thirdparty bins
set(ASSIMP_BIN_DIR ${CMAKE_BINARY_DIR}/thirdParty/assimp/bin/)


add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSIMP_BIN_DIR}/assimp-vc143-mtd.dll"
    "${CMAKE_BINARY_DIR}/src"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSIMP_BIN_DIR}/assimp-vc143-mtd.pdb"
    "${CMAKE_BINARY_DIR}/src"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/src/backend_dx12/backend_dx12d.dll"
    "${CMAKE_BINARY_DIR}/src"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/src/backend_dx12/backend_dx12d.pdb"
    "${CMAKE_BINARY_DIR}/src"
)