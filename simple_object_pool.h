#pragma once
#include <array>
#include <type_traits>
#include <cassert>

namespace pro_game_containers {
    template <class T, uint32_t capacity>
    class simple_object_pool {
    public:
        simple_object_pool() : m_capacity(capacity), m_size(0), m_last_occupied(0) { m_flags.fill(true); }
        auto begin() noexcept {
            return m_pool.begin();
        }
        auto end() noexcept {
            return m_pool.begin() + m_size;
        }
        uint32_t push_back(T element) {
            const uint32_t idx = GetNextFree(m_last_occupied);
            m_pool[idx] = element;
            return m_size++;
        }
        uint32_t push_back() {
            const uint32_t idx = GetNextFree(m_last_occupied);
            m_size++;
            return idx;
        }
        T& operator[](uint32_t idx) {
            assert(idx < m_size);
            return m_pool[idx];
        }
        uint32_t size() const noexcept {
            return m_size;
        }
    private:
        uint32_t GetNextFree(uint32_t start) {
            assert(m_size < m_capacity);
            while(!m_flags[start]){
                if (++start >= m_capacity){
                    start = 0;
                }
            }
            
            m_last_occupied = start;
            m_flags[m_last_occupied] = false;
            return m_last_occupied;
        }
        std::array<T, capacity> m_pool;
        std::array<bool, capacity> m_flags;
        uint32_t m_capacity;
        uint32_t m_size;
        uint32_t m_last_occupied;
    };
}