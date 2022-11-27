#pragma once

#include "imgui.h"
#include <string>

class AppConsole
{
public:
	AppConsole();
	~AppConsole();

	void    Draw(const char* title);
	bool&	Active() { return isActive; }
	void	AddToLog(const std::string& line);
private:
	void    ClearLog();
	void    AddLog(const char* fmt, ...) IM_FMTARGS(2);
	void    ExecCommand(const char* command_line);

	// In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
	static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
	{
		AppConsole* console = (AppConsole*)data->UserData;
		return console->TextEditCallback(data);
	}
	int     TextEditCallback(ImGuiInputTextCallbackData* data);
private:
	bool				  isActive{ false };
	char                  InputBuf[512];
	ImVector<char*>       Items;
	ImVector<const char*> Commands;
	ImVector<char*>       History;
	int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
	ImGuiTextFilter       Filter;
	bool                  AutoScroll;
	bool                  ScrollToBottom;
};