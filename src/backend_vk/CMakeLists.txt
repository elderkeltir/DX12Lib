cmake_minimum_required(VERSION 3.13.0)
project(backend_vk VERSION 0.1.0)

# dllexport
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# definitions
add_definitions(-D_DEBUG)
# add_definitions(-DVK_USE_PLATFORM_XCB_KHR)

# VARs
set(CMAKE_BUILD_PARALLEL_LEVEL 16)
set(CMAKE_MFC_FLAG 2)
set(CMAKE_CXX_STANDARD 17) # no need to manually adjust the CXXFLAGS
set(CMAKE_CXX_STANDARD_REQUIRED on)
set(ROOT_FOLDER ${PROJECT_SOURCE_DIR})
set(THIRD_PARTY_DIR ${ROOT_FOLDER}/../../thirdParty)
set(THIRD_PARTY_IMGUI_DIR ${THIRD_PARTY_DIR}/imgui)
set(VOLK_STATIC_DEFINES VK_USE_PLATFORM_WIN32_KHR)

# source
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

# includes
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../backend_interface)
target_include_directories(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_DIR}/imgui)
target_include_directories(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_DIR}/volk)
target_include_directories(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_DIR}/stb)
target_include_directories(${PROJECT_NAME} PRIVATE ${THIRD_PARTY_DIR}/VulkanMemoryAllocator/include)
target_include_directories(${PROJECT_NAME} PRIVATE "C:\\VulkanSDK\\1.3.268.0\\Include")

# target_link_libraries(${PROJECT_NAME} Vulkan::Vulkan)

# copy files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/src/backend_vk/backend_vkd.dll"
    "${CMAKE_BINARY_DIR}/src"
COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/src/backend_vk/backend_vkd.pdb"
    "${CMAKE_BINARY_DIR}/src"
)