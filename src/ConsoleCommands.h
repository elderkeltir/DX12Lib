#pragma once

#include <string>
#include <vector>

class ConsoleCommands {
public:
	static void ExecuteCommand(std::string name);
	static std::vector<std::string> &GetCommandNames();

private:
	static std::vector<std::string> m_command_names;
};