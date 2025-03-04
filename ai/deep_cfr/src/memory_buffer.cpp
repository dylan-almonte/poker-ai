void StrategyMemory::add(const std::string& info_state, 
                        const std::vector<float>& strategy, 
                        float weight) {
    if (buffer.size() < capacity) {
        buffer.push_back({info_state, strategy, weight});
    } else {
        // Reservoir sampling
        size_t idx = std::uniform_int_distribution<>(0, buffer.size() - 1)(rng);
        if (idx < capacity) {
            buffer[idx] = {info_state, strategy, weight};
        }
    }
}

std::vector<StrategyMemory::Entry> StrategyMemory::sample(int batch_size) {
    std::vector<Entry> batch;
    batch.reserve(std::min(batch_size, static_cast<int>(buffer.size())));
    
    std::vector<size_t> indices(buffer.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::shuffle(indices.begin(), indices.end(), rng);
    
    for (int i = 0; i < std::min(batch_size, static_cast<int>(buffer.size())); i++) {
        batch.push_back(buffer[indices[i]]);
    }
    
    return batch;
} 