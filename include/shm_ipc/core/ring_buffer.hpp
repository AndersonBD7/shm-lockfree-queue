#pragma once
#include <atomic>
#include <cstdint>

namespace shm_ipc {
    template <typename T, size_t Size>
    class RingBuffer {
        // Cache-line aligned indices to prevent false sharing
        alignas(64) std::atomic<uint64_t> write_idx_{0};
        alignas(64) std::atomic<uint64_t> read_idx_{0};
        
        T buffer_[Size];

    public:
        T* alloc() noexcept {
            auto tail = write_idx_.load(std::memory_order_relaxed);
            auto head = read_idx_.load(std::memory_order_acquire);
            if (tail - head >= Size) return nullptr;
            return &buffer_[tail & (Size - 1)];
        }

        void push() noexcept {
            auto tail = write_idx_.load(std::memory_order_relaxed);
            write_idx_.store(tail + 1, std::memory_order_release);
        }
    };
}
