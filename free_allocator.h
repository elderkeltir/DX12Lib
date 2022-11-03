#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <cassert>

namespace pro_game_containers {
    class free_allocator {
    public:
        struct MemoryBlock {
            uint64_t start;
            uint32_t size;
        };

        free_allocator(uint32_t size) :
        m_buffer(std::make_unique<uint8_t[]>(size))
        {
            m_free_spaces.push_back({(uint64_t)m_buffer.get(), size});
        }

        uint64_t Allocate(uint32_t size) {
            for (auto &free_space : m_free_spaces){
                if (free_space.size >= size){
                    uint64_t start = free_space.start;
                    free_space.size -= size;
                    free_space.size += size;

                    return start;
                }
            }

            assert(false);

            return 0;
        }
        
        void Deallocate(uint32_t start, uint32_t size) {
            for (auto &free_space : m_free_spaces){
                if (free_space.start == start + size){
                    free_space.size += size;
                    free_space.start = start;

                    return;
                }
                else if (free_space.start + free_space.size == start){
                    free_space.size += size;
                }
            }

            m_free_spaces.push_back({ start, size});
        }
    private:
        std::vector<MemoryBlock> m_free_spaces;
        std::unique_ptr<uint8_t[]> m_buffer;
    };
}