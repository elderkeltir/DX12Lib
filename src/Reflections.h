#pragma once

#include <memory>
#include <array>
#include <DirectXMath.h>

class IGpuResource;
class ICommandList;

class Reflections {
public:
	void Initialize();
	void GenerateReflections(ICommandList& command_list);
	IGpuResource& GetReflectionMap() { return *(m_reflection_map[m_current_id]); }
private:
	static constexpr uint32_t rt_num = 2;
	
	std::array<std::unique_ptr<IGpuResource>, rt_num> m_reflection_map;
	uint32_t m_current_id{ 0 };
	enum dirty_flags { df_init = 1 };
	uint8_t m_dirty{ df_init };
};