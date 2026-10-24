#pragma once

#include <DnmRHI/Types.hpp>

namespace DnmRHI {
    class Counter {
    public:
        constexpr explicit Counter(const u64 count) noexcept : size(count) {}
    
        class Iterator {
        public:
            constexpr Iterator(const u64 counter) noexcept : counter(counter) {}
            ~Iterator() noexcept = default;
    
            Iterator(const Iterator &other) noexcept {
                counter = other.counter;
            }
    
            constexpr u64 &operator*() noexcept { return counter; }
            constexpr Iterator &operator++() noexcept { ++counter; return *this; }
            constexpr bool operator!=(const Iterator &other) const noexcept { return (other.counter != counter); }
        private:
            u64 counter;
        };
    
        static constexpr Iterator begin() noexcept {
            return {0};
        }
    
        [[nodiscard]] Iterator end() const noexcept {
            return {size};
        }
    private:
        const u64 size;
    };
}