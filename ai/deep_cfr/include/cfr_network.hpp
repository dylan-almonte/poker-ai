#pragma once
#include <torch/torch.h>
#include <vector>
#include <string>
#include <memory>

class CFRNet {
public:
    CFRNet(int input_size, int num_actions);

    std::vector<float> predict(const std::vector<float>& features);
    void train(const std::vector<std::pair<std::string, std::vector<float>>>& batch);
    void save(const std::string& path);
    void load(const std::string& path);

private:
    torch::nn::Sequential model{ nullptr };
    int num_actions;
    torch::optim::Adam* optimizer;

    // Helper method to parse features from info state string
    std::vector<float> parseFeatures(const std::string& info_state);
};

// Value networks (one per player)
class ValueNet {
public:
    ValueNet(int num_players, int input_size, int num_actions);

    std::vector<float> predict(int player_id, const std::vector<float>& features);
    void train(int player_id, const std::vector<std::pair<std::string, std::vector<float>>>& batch);

private:
    std::vector<std::shared_ptr<CFRNet>> networks;
};

// Strategy network
class StrategyNet {
public:
    StrategyNet(int input_size, int num_actions);

    std::vector<float> predict(const std::vector<float>& features);
    void train(const std::vector<std::pair<std::string, std::vector<float>>>& batch);

private:
    std::shared_ptr<CFRNet> network;
};