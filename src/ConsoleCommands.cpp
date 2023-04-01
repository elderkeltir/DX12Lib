#include "ConsoleCommands.h"

#include "WinApplication.h"
#include "Frontend.h"

extern Frontend* gFrontend;


std::vector<std::string> ConsoleCommands::m_command_names;

void ConsoleCommands::ExecuteCommand(std::string name)
{
	if (name == "quit") {
		gFrontend->Close();
	}
	else if (name == "rebuild_shaders") {
		gFrontend->RebuildShaders();
	}
	else if (name.find("r_mode") != std::string::npos) {
		std::string name_copy = name;
		name_copy.erase(0, 7);
		uint32_t r_mode = std::atoi(name_copy.c_str());
		gFrontend->SetRenderMode(r_mode);
	}
}

std::vector<std::string>& ConsoleCommands::GetCommandNames()
{
	if (m_command_names.empty()) {
		m_command_names.push_back("quit");
		m_command_names.push_back("rebuild_shaders");
		m_command_names.push_back("update_constants");
		m_command_names.push_back("r_mode");
	}

	return m_command_names;
}
