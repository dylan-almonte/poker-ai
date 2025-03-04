#pragma once

#include <vector>
#include <string>
#include <memory>
#include <torch/torch.h>

// Simple MLP network for Deep CFR
class MLPImpl : public torch::nn::Module {
public:
    MLPImpl(int input_size, int hidden_size, int output_size);
    torch::Tensor forward(torch::Tensor x);

private:
    torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr};
};

TORCH_MODULE(MLP);

// Neural network wrapper for Deep CFR
class NeuralNet {
private:
    MLP model;
    torch::optim::Adam optimizer;
    int input_size;
    int output_size;
    
public:
    NeuralNet(int input_size, int hidden_size, int output_size, float learning_rate = 0.001);
    
    // Forward pass
    std::vector<float> predict(const std::vector<float>& features);
    
    // Training
    float train(const std::vector<std::vector<float>>& features_batch, 
                const std::vector<std::vector<float>>& targets_batch,
                int batch_size);
    
    // Save and load
    void save(const std::string& path);
    void load(const std::string& path);
}; 