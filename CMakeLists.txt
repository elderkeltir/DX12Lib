cmake_minimum_required(VERSION 3.13.0)
project(DX12Lib-main VERSION 0.1.0)

if(WIN32)
ADD_DEFINITIONS(-D_AFXDLL)
ADD_DEFINITIONS(-DWIN32)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_DEBUG)
ADD_DEFINITIONS(-D_WINDOWS)
set(CMAKE_MFC_FLAG 2)
else()
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
endif()
# ADD_DEFINITIONS(-DUSE_PIX_DEBUG) # for attaching only

# VARs
set(CMAKE_BUILD_PARALLEL_LEVEL 16)
set(CMAKE_CXX_STANDARD 17) # no need to manually adjust the CXXFLAGS
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/thirdParty)

set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "Build as submodule" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/assimp)

if (WIN32)
add_subdirectory(${THIRD_PARTY_DIR}/DirectX-Headers)
set(BUILD_TOOLS OFF CACHE BOOL "Build tex command-line tools" FORCE)
set(BUILD_SAMPLE OFF CACHE BOOL "Build DDSView sample" FORCE)
set(BUILD_DX11 OFF CACHE BOOL "Build with DirectX11 Runtime support" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/DirectXTex)
endif()

add_subdirectory(src)