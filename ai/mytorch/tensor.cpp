#include "mytorch/tensor.hpp"
#include <cmath>
#include <stdexcept>

namespace mytorch {

Tensor Tensor::operator+(const Tensor& other) const {
    if (size() != other.size()) {
        throw std::runtime_error("Tensor sizes don't match for addition");
    }
    
    Tensor result(dims);
    for (size_t i = 0; i < size(); ++i) {
        result[i] = values[i] + other[i];
    }
    return result;
}

Tensor Tensor::operator-(const Tensor& other) const {
    if (size() != other.size()) {
        throw std::runtime_error("Tensor sizes don't match for subtraction");
    }
    
    Tensor result(dims);
    for (size_t i = 0; i < size(); ++i) {
        result[i] = values[i] - other[i];
    }
    return result;
}

Tensor Tensor::operator*(const Tensor& other) const {
    if (size() != other.size()) {
        throw std::runtime_error("Tensor sizes don't match for element-wise multiplication");
    }
    
    Tensor result(dims);
    for (size_t i = 0; i < size(); ++i) {
        result[i] = values[i] * other[i];
    }
    return result;
}

Tensor Tensor::matmul(const Tensor& other) const {
    // Simple matrix multiplication for 2D tensors
    if (dims.size() != 2 || other.dims.size() != 2 || dims[1] != other.dims[0]) {
        throw std::runtime_error("Invalid dimensions for matrix multiplication");
    }
    
    size_t m = dims[0];
    size_t n = other.dims[1];
    size_t k = dims[1];
    
    Tensor result({m, n}, 0.0f);
    
    for (size_t i = 0; i < m; ++i) {
        for (size_t j = 0; j < n; ++j) {
            float sum = 0.0f;
            for (size_t p = 0; p < k; ++p) {
                sum += values[i * k + p] * other[p * n + j];
            }
            result[i * n + j] = sum;
        }
    }
    
    return result;
}

Tensor Tensor::relu() const {
    Tensor result(dims);
    for (size_t i = 0; i < size(); ++i) {
        result[i] = values[i] > 0.0f ? values[i] : 0.0f;
    }
    return result;
}

Tensor Tensor::sigmoid() const {
    Tensor result(dims);
    for (size_t i = 0; i < size(); ++i) {
        result[i] = 1.0f / (1.0f + std::exp(-values[i]));
    }
    return result;
}

Tensor Tensor::tanh() const {
    Tensor result(dims);
    for (size_t i = 0; i < size(); ++i) {
        result[i] = std::tanh(values[i]);
    }
    return result;
}

Tensor Tensor::mse_loss(const Tensor& pred, const Tensor& target) {
    if (pred.size() != target.size()) {
        throw std::runtime_error("Tensor sizes don't match for MSE loss");
    }
    
    float sum = 0.0f;
    for (size_t i = 0; i < pred.size(); ++i) {
        float diff = pred[i] - target[i];
        sum += diff * diff;
    }
    
    Tensor result({1}, sum / pred.size());
    return result;
}

void Tensor::backward() {
    // Simple implementation - in a real system this would compute gradients
    // through the computation graph
}

void Tensor::zero_grad() {
    // Reset gradients to zero
    if (grad) {
        for (size_t i = 0; i < grad->size(); ++i) {
            (*grad)[i] = 0.0f;
        }
    }
}

} // namespace mytorch 