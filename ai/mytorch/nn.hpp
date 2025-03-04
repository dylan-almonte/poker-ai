#pragma once

#include "mytorch/tensor.hpp"
#include <vector>
#include <memory>

namespace mytorch {
namespace nn {

// Sequential container
class Sequential : public Module {
public:
    Sequential() = default;
    
    template<typename... Modules>
    Sequential(Modules&&... modules) {
        (add_module(std::forward<Modules>(modules)), ...);
    }
    
    void add_module(std::shared_ptr<Module> module) {
        layers.push_back(module);
    }
    
    Tensor forward(const Tensor& input) override {
        Tensor output = input;
        for (auto& layer : layers) {
            output = layer->forward(output);
        }
        return output;
    }
    
    void zero_grad() override {
        for (auto& layer : layers) {
            layer->zero_grad();
        }
    }
    
    void update_parameters(float learning_rate) override {
        for (auto& layer : layers) {
            layer->update_parameters(learning_rate);
        }
    }
    
private:
    std::vector<std::shared_ptr<Module>> layers;
};

// ReLU activation
class ReLU : public Module {
public:
    Tensor forward(const Tensor& input) override {
        return input.relu();
    }
    
    void zero_grad() override {}
    void update_parameters(float learning_rate) override {}
};

// Sigmoid activation
class Sigmoid : public Module {
public:
    Tensor forward(const Tensor& input) override {
        return input.sigmoid();
    }
    
    void zero_grad() override {}
    void update_parameters(float learning_rate) override {}
};

} // namespace nn
} // namespace mytorch 