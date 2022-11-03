#pragma once
#include <array>
#include <type_traits>

namespace pro_game_containers {
    class object {
    public:
        virtual ~object() = default;
    protected:
        bool available{true};
    };

    template <class T, uint32_t capacity, class = std::enable_if_t<std::is_base_of_v<object,T>>>
    class simple_object_pool {
    public:
        simple_object_pool() : m_capacity(capacity), m_size(0), m_last_occupied(0) {}
        auto begin() noexcept {
            return m_pool.begin();
        }
        auto end() noexcept {
            return m_pool.begin() + m_size;
        }
        uint32_t push_back(T element) {
            assert(m_size < m_capacity);
            const uint32_t idx = GetNextFree(m_capacity);
            m_pool[idx] = element;
            return m_size++;
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
            while(!m_pool[start].available){
                if (++start >= m_capacity){
                    start = 0;
                }
            }
        }
        std::array<T, capacity> m_pool;
        uint32_t m_capacity;
        uint32_t m_size;
        uint32_t m_last_occupied;
    };
}