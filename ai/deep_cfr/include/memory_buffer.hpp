#pragma once

#include <vector>
#include <random>
#include <algorithm>
#include "info_state.hpp"

class AdvantageMemory {
public:
    struct Entry {
        std::string info_state;
        std::vector<float> advantages;
        float weight;
    };

    void add(const std::string& info_state, const std::vector<float>& advantages, float weight);
    void replace(int idx, const std::string& info_state, const std::vector<float>& advantages, float weight);
    std::vector<Entry> sampleWeighted(int batch_size);

private:
    std::vector<Entry> buffer;
    size_t capacity;
    size_t total_seen_{ 0 };
};

class StrategyMemory {
public:
    StrategyMemory(size_t max_size) : capacity(max_size) {}

    struct Entry {
        std::string info_state;
        std::vector<float> strategy;
        float weight;
    };

    void add(const std::string& info_state, const std::vector<float>& strategy, float weight);
    std::vector<Entry> sample(int batch_size);
    size_t size() const { return buffer.size(); }
    size_t getCapacity() const { return capacity; }

private:
    std::vector<Entry> buffer;
    size_t capacity;
    std::mt19937 rng{ std::random_device{}() };
};