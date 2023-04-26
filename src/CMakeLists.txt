cmake_minimum_required(VERSION 3.13.0)
project(DX12Lib VERSION 0.1.0)

include(CTest)
enable_testing()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
ADD_DEFINITIONS(-D_AFXDLL)
ADD_DEFINITIONS(-DWIN32)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_DEBUG)
ADD_DEFINITIONS(-D_WINDOWS)
set(CMAKE_MFC_FLAG 2)
else()
add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
set(#[[CMAKE_CXX_FLAGS_DEBUG]] "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
include_directories(${THIRD_PARTY_DIR}/glfw/include)
endif()

set(CMAKE_BUILD_PARALLEL_LEVEL 16)
set(CMAKE_CXX_STANDARD 17) # no need to manually adjust the CXXFLAGS
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/../thirdParty)

set(MY_SOURCE_LIST
    main.cpp
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
    Reflections.cpp)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
list(APPEND MY_SOURCE_LIST WinApplication.cpp)
add_executable(${PROJECT_NAME} WIN32 ${MY_SOURCE_LIST})
else()
list(APPEND MY_SOURCE_LIST LinApplication.cpp)
add_executable(${PROJECT_NAME} ${MY_SOURCE_LIST})
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_subdirectory(backend_dx12)
else()
add_subdirectory(backend_vk)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
target_compile_options(${PROJECT_NAME} PUBLIC "/EHsc")
target_link_options(${PROJECT_NAME} PRIVATE "/entry:WinMainCRTStartup")
target_link_libraries(${PROJECT_NAME} backend_dx12)
else()
target_link_libraries(${PROJECT_NAME} backend_vk)
endif()

target_include_directories(${PROJECT_NAME}  PRIVATE ${THIRD_PARTY_DIR}/rapidjson/include)
target_include_directories(${PROJECT_NAME}  PRIVATE ${PROJECT_SOURCE_DIR}/backend_interface)
target_include_directories(${PROJECT_NAME}  PRIVATE ${THIRD_PARTY_DIR}/assimp/include)
target_include_directories(${PROJECT_NAME}  PRIVATE ${CMAKE_BINARY_DIR}/thirdParty/assimp/include)
target_include_directories(${PROJECT_NAME}  PRIVATE ${THIRD_PARTY_DIR}/DirectXMath/Inc)

if (CMAKE_SYSTEM_NAME STREQUAL "Windows")
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
else()
find_package(assimp)
target_link_libraries(${PROJECT_NAME} dl)
target_link_libraries(${PROJECT_NAME} pthread)
target_link_libraries(${PROJECT_NAME} glfw)
if (CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
target_link_libraries(${PROJECT_NAME} ${ASSIMP_LIBRARIES})
else()
target_link_libraries(${PROJECT_NAME} assimp)
endif()
if (NOT CMAKE_SYSTEM_NAME STREQUAL "Windows")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND /bin/bash ${ROOT_FOLDER}/../generate_compile_json.sh
)
endif()
endif()
