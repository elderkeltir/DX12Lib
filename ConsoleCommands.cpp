#include "ConsoleCommands.h"

#include "Application.h"
#include "DXAppImplementation.h"

extern DXAppImplementation* gD3DApp;


std::vector<std::string> ConsoleCommands::m_command_names;

void ConsoleCommands::ExecuteCommand(std::string name)
{
	if (name == "quit") {
		Application::Close();
	}
	else if (name == "rebuild_shaders") {
		gD3DApp->RebuildShaders();
	}
}

std::vector<std::string>& ConsoleCommands::GetCommandNames()
{
	if (m_command_names.empty()) {
		m_command_names.push_back("quit");
		m_command_names.push_back("rebuild_shaders");
		m_command_names.push_back("update_constants");
	}

	return m_command_names;
}
