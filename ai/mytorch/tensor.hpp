#pragma once

#include <vector>
#include <memory>
#include <random>
#include <algorithm>
#include <functional>
#include <iostream>
#include <cmath>

namespace mytorch {

// Forward declarations
class Tensor;
class Module;

// Simple tensor class
class Tensor {
public:
    // Constructors
    Tensor() = default;
    
    // Create tensor from vector
    template<typename T>
    Tensor(const std::vector<T>& data, const std::vector<size_t>& shape) {
        reshape(shape);
        size_t total_size = size();
        values.resize(total_size);
        for (size_t i = 0; i < std::min(total_size, data.size()); ++i) {
            values[i] = static_cast<float>(data[i]);
        }
    }
    
    // Create tensor with given shape and fill with value
    Tensor(const std::vector<size_t>& shape, float value = 0.0f) {
        reshape(shape);
        values.resize(size(), value);
    }
    
    // Reshape tensor
    void reshape(const std::vector<size_t>& new_shape) {
        dims = new_shape;
        requires_grad = false;
        grad = nullptr;
    }
    
    // Get total size
    size_t size() const {
        if (dims.empty()) return 0;
        size_t total = 1;
        for (auto d : dims) total *= d;
        return total;
    }
    
    // Get shape
    const std::vector<size_t>& shape() const { return dims; }
    
    // Access elements
    float& operator[](size_t idx) { return values[idx]; }
    const float& operator[](size_t idx) const { return values[idx]; }
    
    // Basic operations
    Tensor operator+(const Tensor& other) const;
    Tensor operator-(const Tensor& other) const;
    Tensor operator*(const Tensor& other) const;  // Element-wise multiplication
    Tensor matmul(const Tensor& other) const;     // Matrix multiplication
    
    // Activation functions
    Tensor relu() const;
    Tensor sigmoid() const;
    Tensor tanh() const;
    
    // Loss functions
    static Tensor mse_loss(const Tensor& pred, const Tensor& target);
    
    // Gradient operations
    void backward();
    void zero_grad();
    
    // Convert to vector
    std::vector<float> to_vector() const { return values; }
    
    // Print tensor
    void print() const {
        std::cout << "Tensor(shape=[";
        for (size_t i = 0; i < dims.size(); ++i) {
            std::cout << dims[i];
            if (i < dims.size() - 1) std::cout << ", ";
        }
        std::cout << "], values=[";
        for (size_t i = 0; i < std::min(values.size(), size_t(10)); ++i) {
            std::cout << values[i];
            if (i < values.size() - 1) std::cout << ", ";
        }
        if (values.size() > 10) std::cout << "...";
        std::cout << "])" << std::endl;
    }
    
    // Get a single value (for scalar tensors)
    template<typename T = float>
    T item() const {
        if (values.empty()) return T(0);
        return static_cast<T>(values[0]);
    }

private:
    std::vector<size_t> dims;
    std::vector<float> values;
    bool requires_grad = false;
    std::shared_ptr<Tensor> grad;
};

// Base module class
class Module {
public:
    virtual ~Module() = default;
    virtual Tensor forward(const Tensor& input) = 0;
    virtual void zero_grad() = 0;
    virtual void update_parameters(float learning_rate) = 0;
};

// Linear layer
class Linear : public Module {
public:
    Linear(size_t in_features, size_t out_features) 
        : weights(std::vector<size_t>{out_features, in_features}, 0.0f),
          bias(std::vector<size_t>{out_features}, 0.0f) {
        // Initialize weights with Xavier/Glorot initialization
        std::random_device rd;
        std::mt19937 gen(rd());
        float limit = std::sqrt(6.0f / (in_features + out_features));
        std::uniform_real_distribution<float> dist(-limit, limit);
        
        for (size_t i = 0; i < weights.size(); ++i) {
            weights[i] = dist(gen);
        }
    }
    
    Tensor forward(const Tensor& input) override {
        return input.matmul(weights) + bias;
    }
    
    void zero_grad() override {
        // Implementation for backpropagation
    }
    
    void update_parameters(float learning_rate) override {
        // Implementation for optimizer step
    }
    
private:
    Tensor weights;
    Tensor bias;
};

// Simple optimizer
class Adam {
public:
    Adam(std::vector<std::reference_wrapper<Module>> modules, float lr = 0.001f, 
         float beta1 = 0.9f, float beta2 = 0.999f, float eps = 1e-8f)
        : modules(modules), learning_rate(lr), beta1(beta1), beta2(beta2), epsilon(eps) {}
    
    void zero_grad() {
        for (auto& module : modules) {
            module.get().zero_grad();
        }
    }
    
    void step() {
        for (auto& module : modules) {
            module.get().update_parameters(learning_rate);
        }
    }
    
private:
    std::vector<std::reference_wrapper<Module>> modules;
    float learning_rate;
    float beta1;
    float beta2;
    float epsilon;
};

} // namespace mytorch 