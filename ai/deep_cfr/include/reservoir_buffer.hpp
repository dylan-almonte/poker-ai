#pragma once

#include <vector>
#include <random>
#include <numeric>
#include "info_state.hpp"

// Memory structure for advantage updates
struct AdvantageMemory {
    InfoState info_state;
    std::vector<float> advantages;
    float reach_prob;
    
    AdvantageMemory(const InfoState& info_state, 
                    const std::vector<float>& advantages,
                    float reach_prob)
        : info_state(info_state), advantages(advantages), reach_prob(reach_prob) {}
};

// Memory structure for strategy updates
struct StrategyMemory {
    InfoState info_state;
    std::vector<float> strategy;
    float weight;
    
    StrategyMemory(const InfoState& info_state,
                   const std::vector<float>& strategy,
                   float weight)
        : info_state(info_state), strategy(strategy), weight(weight) {}
};

// Reservoir sampling buffer for Deep CFR
template <typename T>
class ReservoirBuffer {
private:
    std::vector<T> buffer;
    size_t capacity;
    size_t count;
    mutable std::mt19937 rng; // Make mutable to allow modification in const methods
    
public:
    ReservoirBuffer(size_t capacity) 
        : capacity(capacity), count(0), rng(std::random_device{}()) {}
    
    // Add an item to the buffer using reservoir sampling
    void add(const T& item) {
        if (buffer.size() < capacity) {
            buffer.push_back(item);
        } else {
            // Reservoir sampling
            size_t j = std::uniform_int_distribution<size_t>(0, count)(rng);
            if (j < capacity) {
                buffer[j] = item;
            }
        }
        count++;
    }
    
    // Get a random batch of items
    std::vector<T> sample(size_t batch_size) const {
        std::vector<T> batch;
        if (buffer.empty()) return batch;
        
        size_t actual_batch_size = std::min(batch_size, buffer.size());
        std::vector<size_t> indices(buffer.size());
        std::iota(indices.begin(), indices.end(), 0);
        
        // Use a non-const copy of the RNG for shuffling
        auto rng_copy = rng;
        std::shuffle(indices.begin(), indices.end(), rng_copy);
        
        batch.reserve(actual_batch_size);
        for (size_t i = 0; i < actual_batch_size; i++) {
            batch.push_back(buffer[indices[i]]);
        }
        
        return batch;
    }
    
    // Get the current size of the buffer
    size_t size() const {
        return buffer.size();
    }
    
    // Clear the buffer
    void clear() {
        buffer.clear();
        count = 0;
    }
}; 