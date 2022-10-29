#pragma once
#include <array>

namespace pro_game_containers {
    template <typename T, uint32_t capacity>
    class simple_object_pool {
    public:
        simple_object_pool() : m_capacity(capacity), m_size(0) {}
        auto begin() {
            return m_pool.begin();
        }
        auto end() {
            return m_pool.begin() + m_size;
        }
        uint32_t push_back(T element) {
            m_pool[m_size] = element;
            return m_size++;
        }
        T& operator[](uint32_t idx) {
            return m_pool[idx];
        }
        uint32_t size() const {
            return m_size;
        }
    private:
        std::array<T, capacity> m_pool;
        uint32_t m_capacity;
        uint32_t m_size;
    };
}