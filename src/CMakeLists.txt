cmake_minimum_required(VERSION 3.13.0)
project(DX12Lib VERSION 0.1.0)

# definitions
ADD_DEFINITIONS(-D_AFXDLL)
ADD_DEFINITIONS(-DWIN32)
ADD_DEFINITIONS(-D_UNICODE)
ADD_DEFINITIONS(-DUNICODE)
ADD_DEFINITIONS(-D_DEBUG)
ADD_DEFINITIONS(-D_WINDOWS)
set(CMAKE_MFC_FLAG 2)

 # VARS
set(CMAKE_BUILD_PARALLEL_LEVEL 16)
set(CMAKE_CXX_STANDARD 17) # no need to manually adjust the CXXFLAGS
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(ROOT_FOLDER ${CMAKE_CURRENT_SOURCE_DIR})
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/../thirdParty)

# source
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
    Reflections.cpp
    WinApplication.cpp)

# create executable
add_executable(${PROJECT_NAME} ${MY_SOURCE_LIST})

# add dependancy
add_subdirectory(backend_dx12)
add_subdirectory(backend_vk)

# includes
target_include_directories(${PROJECT_NAME}  PRIVATE ${THIRD_PARTY_DIR}/rapidjson/include)
target_include_directories(${PROJECT_NAME}  PRIVATE ${PROJECT_SOURCE_DIR}/backend_interface)
target_include_directories(${PROJECT_NAME}  PRIVATE ${THIRD_PARTY_DIR}/DirectXMath/Inc)

# link libs
target_compile_options(${PROJECT_NAME} PUBLIC "/EHsc")
target_link_options(${PROJECT_NAME} PRIVATE "/entry:WinMainCRTStartup")
target_link_libraries(${PROJECT_NAME} backend_dx12)
target_link_libraries(${PROJECT_NAME} backend_vk)

# copy files
set(ASSIMP_BIN_DIR ${CMAKE_BINARY_DIR}/thirdParty/assimp/bin/)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSIMP_BIN_DIR}/assimp-vc143-mtd.dll"
    "${CMAKE_BINARY_DIR}/src"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${ASSIMP_BIN_DIR}/assimp-vc143-mtd.pdb"
    "${CMAKE_BINARY_DIR}/src"
)
target_link_libraries(${PROJECT_NAME} assimp)
