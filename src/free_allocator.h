#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include <cassert>

namespace pro_game_containers {
    // TODO: ideally introduce Pages to be able to extend/shrink the buffer if necessary?
    class free_allocator {
    public:
        free_allocator(uint32_t size) :
        m_buffer(std::make_unique<uint8_t[]>(size))
        {
            m_free_spaces.push_back({(uint64_t)m_buffer.get(), size});
        }

        uint64_t allocate(uint32_t size) {
            for (auto &free_space : m_free_spaces){
                if (free_space.size >= size){
                    uint64_t start = free_space.start;
                    free_space.size -= size;
                    free_space.start += size;

                    return start;
                }
            }

            assert(false);

            return 0;
        }
        
        void deallocate(uint64_t start, uint32_t size) {
            std::vector<MemoryBlock>::iterator before_block = m_free_spaces.end();
            std::vector<MemoryBlock>::iterator after_block = m_free_spaces.end();

            for (auto &it = m_free_spaces.begin(); it != m_free_spaces.end(); ++it){
                if (it->start == start + size){
                    after_block = it;
                }
                else if (it->start + it->size == start){
                    before_block = it;
                }
            }

            if (before_block != m_free_spaces.end() && after_block != m_free_spaces.end()){
                before_block->size += (after_block->size + size);
                m_free_spaces.erase(after_block);
            }
            else if (before_block != m_free_spaces.end()) {
                after_block->size += size;
                after_block->start = start;
            }
            else if (after_block != m_free_spaces.end()) {
                before_block->size += size;
            }
            else {
                m_free_spaces.push_back({ start, size});
            }
        }

        uint64_t begin() const { return (uint64_t)m_buffer.get(); }
    private:
        struct MemoryBlock {
            uint64_t start;
            uint32_t size;
        };
        std::vector<MemoryBlock> m_free_spaces;
        std::unique_ptr<uint8_t[]> m_buffer;
    };
}