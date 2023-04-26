#include "LinApplication.h"

#ifndef WIN32
//#define FreeBSD
#include "Frontend.h"
#ifndef __FreeBSD__
#include <unistd.h>
#else
#include <sys/types.h>
#include <sys/sysctl.h>
#include <cstddef>
#endif
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include <iostream>


#ifndef MAX_PATH
#define MAX_PATH 256
#endif

Frontend::KeyboardButton CastKey(int key) {
    Frontend::KeyboardButton btn{ Frontend::KeyboardButton::kb_none };
    switch (key) {
    case GLFW_KEY_W:
        btn = Frontend::KeyboardButton::kb_w;
        break;
    case GLFW_KEY_A:
        btn = Frontend::KeyboardButton::kb_a;
        break;
    case GLFW_KEY_S:
        btn = Frontend::KeyboardButton::kb_s;
        break;
    case GLFW_KEY_D:
        btn = Frontend::KeyboardButton::kb_d;
        break;
//    case VK_OEM_3:
//        btn = Frontend::KeyboardButton::kb_tilda;
//        break;
    default:
        break;
    }

    return btn;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Frontend * frontend = (Frontend*)glfwGetWindowUserPointer(window);

    if(!frontend)
        return;


    if (key == GLFW_KEY_W){
        if (action == GLFW_PRESS)
            frontend->OnKeyDown(CastKey(key));
        else if (action == GLFW_RELEASE)
            frontend->OnKeyUp(CastKey(key));
    }
    if (key == GLFW_KEY_S){
        if (action == GLFW_PRESS)
            frontend->OnKeyDown(CastKey(key));
        else if (action == GLFW_RELEASE)
            frontend->OnKeyUp(CastKey(key));
    }
    if (key == GLFW_KEY_D){
        if (action == GLFW_PRESS)
            frontend->OnKeyDown(CastKey(key));
        else if (action == GLFW_RELEASE)
            frontend->OnKeyUp(CastKey(key));
    }
    if (key == GLFW_KEY_A){
        if (action == GLFW_PRESS)
            frontend->OnKeyDown(CastKey(key));
        else if (action == GLFW_RELEASE)
            frontend->OnKeyUp(CastKey(key));
    }
}
bool firstMouse = true; // TODO: rewrite all this code later. I'm in hurry now QQ
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
    Frontend * frontend = (Frontend*)glfwGetWindowUserPointer(window);
    if(!frontend)
        return;

    Frontend::MouseButton btn{ Frontend::MouseButton::m_undefined };
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        btn = Frontend::MouseButton::mb_right;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        btn = Frontend::MouseButton::mb_left;
    }
    else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
        btn = Frontend::MouseButton::mb_middle;
    }

    frontend->OnMouseMoved(btn, xpos, ypos);
}


VkSurfaceKHR CreateSurface(VkInstance instance, GLFWwindow* window) {
    VkSurfaceKHR surf;
    glfwCreateWindowSurface(instance, window, NULL, &surf);

    return surf;
}

LinApplication::~LinApplication() {

}

int LinApplication::Run() {
    // Window
	assert(glfwInit());

    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWwindow* window = glfwCreateWindow(1280, 720, "DX12LibApp", NULL, NULL);

    // Initialize the sample. OnInit is defined in each child-implementation of DXApp.
    WindowHandler w_hndl;
    w_hndl.ptr = (uint64_t)window;

    uint32_t glfwExtensionsNum = 0;
    const char ** glfwxtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionsNum);
    std::vector<const char*> extensions(glfwExtensionsNum);
    for (size_t i = 0u; i < glfwExtensionsNum; i++){
        extensions[i] = glfwxtensions[i];
    }
    w_hndl.extensions = extensions;

    w_hndl.callback = (void*)&CreateSurface;


    std::filesystem::path root_dir;

    // find absolute path
    #ifndef __FreeBSD__
    char buff[MAX_PATH];
    ssize_t len = ::readlink("/proc/self/exe", buff, sizeof(buff)-1);
    if (len != -1) {
      buff[len] = '\0';
      std::filesystem::path p = std::filesystem::path(buff);
      root_dir = p.parent_path().parent_path().parent_path();

    }
    #else
    int mib[4] = {CTL_KERN, KERN_PROC, KERN_PROC_PATHNAME, -1};
    char buff[MAX_PATH];
    size_t cb = sizeof(buff);
    sysctl(mib, 4, buff, &cb, NULL, 0);
    std::filesystem::path p = std::filesystem::path(buff);
    root_dir = p.parent_path().parent_path().parent_path();
    #endif
// Frontend
    m_frontend->OnInit(w_hndl, root_dir);


    // GLFW
    glfwSetWindowUserPointer(window, m_frontend.get());

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);


	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		
        m_frontend->OnUpdate();
        m_frontend->OnRender();
	}

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

LinApplication::LinApplication(uint32_t width, uint32_t height, const std::wstring& window_name) :
    m_width(width),
    m_height(height),
    m_window_name(window_name),
    m_frontend(new Frontend(width, height, window_name))
{

}

#endif // WIN32
