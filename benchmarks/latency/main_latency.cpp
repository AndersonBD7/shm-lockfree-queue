#include "../../include/shm_ipc/core/ring_buffer.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <algorithm>
#include <numeric>
#include <iomanip>

using namespace shm_ipc::core;

struct Payload {
    uint64_t id;
    uint64_t ts_sent;
    char data[48]; // Make it 64 bytes total
};

constexpr size_t OPS = 10'000'000;
constexpr size_t Q_SIZE = 1024;

void print_stats(const std::vector<double>& latencies) {
    auto sorted = latencies;
    std::sort(sorted.begin(), sorted.end());

    std::cout << "\n[Latency Statistics (ns)]\n";
    std::cout << "-------------------------\n";
    std::cout << "  Min: " << sorted.front() << "\n";
    std::cout << "  P50: " << sorted[sorted.size() * 0.50] << "\n";
    std::cout << "  P99: " << sorted[sorted.size() * 0.99] << "\n";
    std::cout << "  P99.9: " << sorted[sorted.size() * 0.999] << "\n";
    std::cout << "  Max: " << sorted.back() << "\n";
    
    double sum = std::accumulate(sorted.begin(), sorted.end(), 0.0);
    std::cout << "  Avg: " << sum / sorted.size() << "\n";
}

int main() {
    auto* ring = new RingBuffer<Payload, Q_SIZE>();
    std::atomic<bool> running{true};
    std::vector<double> latencies;
    latencies.reserve(OPS);

    std::thread consumer([&]() {
        while(running) {
            const auto* ptr = ring->peek();
            if (ptr) {
                // Emulate minimal processing
                ring->pop();
            } else {
                SHM_PAUSE(); // CPU Yield hint
            }
        }
    });

    std::cout << "[*] Warming up..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "[*] Running " << OPS << " iterations..." << std::endl;

    for(size_t i=0; i<OPS; ++i) {
        auto t1 = std::chrono::steady_clock::now();
        
        while(true) {
            auto* slot = ring->claim_for_write();
            if (slot) {
                slot->id = i;
                ring->commit_write();
                break;
            }
            SHM_PAUSE();
        }
        
        // In a real SPSC latency test, we would measure RTT. 
        // Here we measure write overhead for demo.
        auto t2 = std::chrono::steady_clock::now();
        latencies.push_back(std::chrono::duration<double, std::nano>(t2 - t1).count());
    }

    running = false;
    consumer.join();

    print_stats(latencies);
    delete ring;
    return 0;
}