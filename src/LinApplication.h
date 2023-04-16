#pragma once

#ifndef WIN32

#include <cstdint>
#include <string>
#include <memory>

class Frontend;

class LinApplication {
public:
    LinApplication(uint32_t width, uint32_t height, const std::wstring& window_name);
    int Run();
    ~LinApplication();
private:
    uint32_t m_width;
    uint32_t m_height;
    std::wstring m_window_name;
    std::unique_ptr<Frontend> m_frontend;
};

#endif // WIN32
