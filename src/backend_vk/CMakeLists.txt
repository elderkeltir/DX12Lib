cmake_minimum_required(VERSION 3.13.0)
project(backend_vk VERSION 0.1.0)

include(CTest)
enable_testing()

ADD_DEFINITIONS(-D_DEBUG)

add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
set(#[[CMAKE_CXX_FLAGS_DEBUG]] "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -Wno-nullability-completeness")
set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_XLIB_KHR)

find_package(PkgConfig REQUIRED)
find_package(Vulkan REQUIRED)

# VARs
set(CMAKE_BUILD_PARALLEL_LEVEL 16)
set(CMAKE_MFC_FLAG 2)
set(CMAKE_CXX_STANDARD 17) # no need to manually adjust the CXXFLAGS
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/../../thirdParty)
set(THIRD_PARTY_IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/../backend_interface)
include_directories(${THIRD_PARTY_DIR}/imgui)
include_directories(${THIRD_PARTY_DIR}/volk)
include_directories(${THIRD_PARTY_DIR}/stb)
include_directories(${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include)

add_library(${PROJECT_NAME} SHARED
    # volk
    ${THIRD_PARTY_DIR}/volk/volk.c
    # imgui
    ${THIRD_PARTY_IMGUI_DIR}/imgui.cpp
    ${THIRD_PARTY_IMGUI_DIR}/imgui_draw.cpp
    ${THIRD_PARTY_IMGUI_DIR}/imgui_tables.cpp
    ${THIRD_PARTY_IMGUI_DIR}/imgui_widgets.cpp
    #${THIRD_PARTY_IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
    #${THIRD_PARTY_IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    #
    VkBackend.cpp
    CommandList.cpp
    CommandQueue.cpp
    Fence.cpp
    HeapBuffer.cpp
    VkMemoryHelper.cpp
    VkDevice.cpp
    ResourceDescriptor.cpp
    DescriptorHeapCollection.cpp
    GpuResource.cpp
    #Console.cpp
    #ConsoleCommands.cpp
    SwapChain.cpp
    ShaderManager.cpp
    TextureLoader.cpp
#    ImguiHelper.cpp
    RootSignature.cpp
    DynamicGpuHeap.cpp
    Techniques.cpp
)


target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

target_link_libraries(${PROJECT_NAME} dl)
target_link_libraries(${PROJECT_NAME} pthread)
