#pragma once

#include <memory>
#include <directx/d3dx12.h>
#include <DirectXMath.h>

class GpuResource;
class CommandQueue;
class CommandList;

class Reflections {
public:
	void Initialize();
	void GenerateReflections(CommandList& command_list);
	GpuResource& GetReflectionMap() { return m_reflection_map[m_current_id]; }
private:
	static constexpr uint32_t rt_num = 2;
	
	std::unique_ptr<GpuResource[]> m_reflection_map;
	uint32_t m_current_id{ 0 };
	enum dirty_flags { df_init = 1 };
	uint8_t m_dirty{ df_init };
};