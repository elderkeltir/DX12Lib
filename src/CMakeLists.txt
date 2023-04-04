cmake_minimum_required(VERSION 3.13.0)
project(DX12Lib VERSION 0.1.0)

include(CTest)
enable_testing()

if(WIN32)
ADD_DEFINITIONS(-D_AFXDLL)
ADD_DEFINITIONS(-DWIN32)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_DEBUG)
ADD_DEFINITIONS(-D_WINDOWS)
set(CMAKE_MFC_FLAG 2)
else()
add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
include_directories(${THIRD_PARTY_DIR}/glfw/include)
endif()

set(CMAKE_BUILD_PARALLEL_LEVEL 16)
set(CMAKE_CXX_STANDARD 17) # no need to manually adjust the CXXFLAGS
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/../thirdParty)

if(WIN32)
add_executable(${PROJECT_NAME}  WIN32 main.cpp
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
else()
add_executable(${PROJECT_NAME}   main.cpp
    LinApplication.cpp
    # FileManager.cpp
    # Frontend.cpp
    # RenderModel.cpp
    # ResourceManager.cpp
    # Transformations.cpp
    # Level.cpp
    # FreeCamera.cpp
    # LevelEntity.cpp
    # ConstantBufferManager.cpp
    # RenderQuad.cpp
    # RenderObject.cpp
    # GpuDataManager.cpp
    # MaterialManager.cpp
    # RenderMesh.cpp
    # SkyBox.cpp
    # SSAO.cpp
    # Plane.cpp
    # Sun.cpp
    # Reflections.cpp
)
endif()

if(WIN32)
add_subdirectory(backend_dx12)
else()
add_subdirectory(backend_vk)
endif()

if (WIN32)
target_compile_options(${PROJECT_NAME} PUBLIC "/EHsc")
target_link_options(${PROJECT_NAME} PRIVATE "/entry:WinMainCRTStartup")
target_link_libraries(${PROJECT_NAME} backend_dx12)
else()
target_link_libraries(${PROJECT_NAME} backend_vk)
find_package(PkgConfig REQUIRED)
find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)
endif()

target_include_directories(${PROJECT_NAME}  PUBLIC ${THIRD_PARTY_DIR}/rapidjson/include)
target_include_directories(${PROJECT_NAME}  PUBLIC ${PROJECT_SOURCE_DIR}/backend_interface)
target_include_directories(${PROJECT_NAME}  PUBLIC ${THIRD_PARTY_DIR}/assimp/include)
target_include_directories(${PROJECT_NAME}  PUBLIC ${CMAKE_BINARY_DIR}/thirdParty/assimp/include)
target_include_directories(${PROJECT_NAME}  PUBLIC ${THIRD_PARTY_DIR}/DirectXMath/Inc)

target_link_libraries(${PROJECT_NAME} assimp)

if (WIN32)
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
else()
target_link_libraries(${PROJECT_NAME} dl)
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} glfw)
endif()