#pragma once

#ifndef WIN32

#include <cstdint>
#include <string>

class LinApplication {
public:
    LinApplication(uint32_t width, uint32_t height, const std::wstring& window_name);
    int Run();
private:
    uint32_t m_width;
    uint32_t m_height;
    std::wstring m_window_name;
};

#endif // WIN32