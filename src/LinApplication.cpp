#include "LinApplication.h"

int LinApplication::Run() {
    return 0;
}

LinApplication::LinApplication(uint32_t width, uint32_t height, const std::wstring& window_name) :
    m_width(width),
    m_height(height),
    m_window_name(window_name)
{

}