#pragma once

#pragma warning(disable:4996) // _CRT_SECURE_NO_WARNINGS 

#include <string>
#include <fstream>
#include <ctime>
#include <chrono> 
#include <assert.h>

typedef void (*FuncPtrVoidStr)(const std::string&);

class logger {
public:
	enum log_level
	{
		ll_CRITICAL,
		ll_ERROR,
		ll_WARNING,
		ll_DEBUG,
		ll_INFO
	};
	logger(const std::string& file_name, log_level level) : m_flushing_buffer_size(LINE_SIZE), m_log_level_needed(level)
	{
		m_fileStream.open(file_name, std::ofstream::out | std::ofstream::trunc);
		assert(m_fileStream.is_open());

	}
	~logger()
	{
		if (!m_buffer.empty())
		{
			m_fileStream << m_buffer;
		}
		m_fileStream.close();
	}
	const std::string timenow()
	{
		time_t timenow = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		std::string time_now = ctime(&timenow);
		time_now.erase(time_now.size() - 1);
		return  time_now;
	}

	template<typename... Ts>
	void hlog(log_level level, const std::string& line, Ts ... args)
	{
		if (level <= m_log_level_needed)
		{
			const int buffer_size = LINE_SIZE;
			char buffer[buffer_size];
			snprintf(buffer, buffer_size, line.c_str(), args...);
			std::string text_line = buffer;
			text_line = "[" + timenow()+ "][" + log_level_str(level) +"] " + text_line + "\n";
			
			if (m_console_cb)
				m_console_cb(text_line);

			m_buffer = m_buffer + text_line;
			if (m_buffer.length() >= m_flushing_buffer_size)
			{
				m_fileStream << m_buffer;
				m_buffer.clear();
			}
		}
	}

	void SetConsoleCb(FuncPtrVoidStr cb) {
		m_console_cb = cb;
	}

private:
	std::string log_level_str(log_level level) {
		switch (level) {
		case log_level::ll_CRITICAL:
			return "CRITICAL";
			break;
		case log_level::ll_ERROR:
			return "ERROR";
			break;
		case log_level::ll_WARNING:
			return "WARNING";
			break;
		case log_level::ll_DEBUG:
			return "DEBUG";
			break;
		case log_level::ll_INFO:
			return "INFO";
			break;
		}
	}
	static const uint32_t LINE_SIZE = 512;
	std::ofstream m_fileStream;
	std::string m_buffer;
	const size_t m_flushing_buffer_size;
	log_level m_log_level_needed;
	FuncPtrVoidStr m_console_cb{ nullptr };
};
