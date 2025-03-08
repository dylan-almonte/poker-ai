#include "cfr_neural_net.hpp"
#include <iostream>

MLPImpl::MLPImpl(int input_size, int hidden_size, int output_size) {
    fc1 = register_module("fc1", torch::nn::Linear(input_size, hidden_size));
    fc2 = register_module("fc2", torch::nn::Linear(hidden_size, hidden_size));
    fc3 = register_module("fc3", torch::nn::Linear(hidden_size, output_size));
}

torch::Tensor MLPImpl::forward(torch::Tensor x) {
    x = torch::relu(fc1->forward(x));
    x = torch::relu(fc2->forward(x));
    x = fc3->forward(x);
    return x;
}

NeuralNet::NeuralNet(int input_size, int hidden_size, int output_size, float learning_rate)
    : model(input_size, hidden_size, output_size),
      optimizer(model->parameters(), torch::optim::AdamOptions(learning_rate)),
      input_size(input_size),
      output_size(output_size) {
    
    // Determine the best device to use
    torch::Device device(torch::kCPU);
    
    // Check for MPS (Metal Performance Shaders) support on M1 Macs
    #if defined(APPLE_SILICON) && defined(TORCH_ENABLE_MPS)
    if (torch::mps::is_available()) {
        std::cout << "MPS is available! Training on Apple Silicon GPU." << std::endl;
        device = torch::Device(torch::kMPS);
    } else 
    #endif
    // Fall back to CUDA if available (not on M1 Macs)
    if (torch::cuda::is_available()) {
        std::cout << "CUDA is available! Training on NVIDIA GPU." << std::endl;
        device = torch::Device(torch::kCUDA);
    } else {
        std::cout << "Using CPU for training." << std::endl;
    }
    
    model->to(device);
}

std::vector<float> NeuralNet::predict(const std::vector<float>& features) {
    // Convert features to tensor
    torch::Tensor input = torch::tensor(features, torch::kFloat32).reshape({1, -1});
    
    // Move to same device as model
    input = input.to(model->parameters().begin()->device());
    
    // Forward pass
    torch::NoGradGuard no_grad;
    torch::Tensor output = model->forward(input);
    
    // Convert to vector
    std::vector<float> result;
    result.resize(output.numel());
    
    // Copy data to vector (safer than using data_ptr on different devices)
    auto output_cpu = output.to(torch::kCPU);
    std::memcpy(result.data(), output_cpu.data_ptr<float>(), 
                output_cpu.numel() * sizeof(float));
    
    return result;
}

float NeuralNet::train(const std::vector<std::vector<float>>& features_batch, 
                      const std::vector<std::vector<float>>& targets_batch,
                      int batch_size) {
    // Get device
    auto device = model->parameters().begin()->device();
    
    // Convert to tensors
    std::vector<torch::Tensor> inputs;
    std::vector<torch::Tensor> targets;
    
    for (size_t i = 0; i < features_batch.size(); i++) {
        inputs.push_back(torch::tensor(features_batch[i], torch::kFloat32));
        targets.push_back(torch::tensor(targets_batch[i], torch::kFloat32));
    }
    
    torch::Tensor inputs_tensor = torch::stack(inputs);
    torch::Tensor targets_tensor = torch::stack(targets);
    
    // Move to device
    inputs_tensor = inputs_tensor.to(device);
    targets_tensor = targets_tensor.to(device);
    
    // Training loop
    float total_loss = 0.0f;
    int num_batches = (features_batch.size() + batch_size - 1) / batch_size;
    
    for (int i = 0; i < num_batches; i++) {
        // Get batch
        int start_idx = i * batch_size;
        int end_idx = std::min(start_idx + batch_size, static_cast<int>(features_batch.size()));
        torch::Tensor batch_inputs = inputs_tensor.slice(0, start_idx, end_idx);
        torch::Tensor batch_targets = targets_tensor.slice(0, start_idx, end_idx);
        
        // Forward pass
        optimizer.zero_grad();
        torch::Tensor outputs = model->forward(batch_inputs);
        
        // Compute loss (MSE)
        torch::Tensor loss = torch::mse_loss(outputs, batch_targets);
        
        // Backward pass
        loss.backward();
        optimizer.step();
        
        total_loss += loss.item<float>();
    }
    
    return total_loss / num_batches;
}

void NeuralNet::save(const std::string& path) {
    // Move model to CPU before saving
    torch::Device cpu_device(torch::kCPU);
    model->to(cpu_device);
    torch::save(model, path);
    
    // Move model back to original device
    auto device = torch::kCPU;
    #if defined(APPLE_SILICON) && defined(TORCH_ENABLE_MPS)
    if (torch::mps::is_available()) {
        device = torch::kMPS;
    } else 
    #endif
    if (torch::cuda::is_available()) {
        device = torch::kCUDA;
    }
    model->to(torch::Device(device));
}

void NeuralNet::load(const std::string& path) {
    // Load to CPU first
    torch::load(model, path);
    
    // Then move to the appropriate device
    torch::Device device(torch::kCPU);
    
    #if defined(APPLE_SILICON) && defined(TORCH_ENABLE_MPS)
    if (torch::mps::is_available()) {
        device = torch::Device(torch::kMPS);
    } else 
    #endif
    if (torch::cuda::is_available()) {
        device = torch::Device(torch::kCUDA);
    }
    
    model->to(device);
} 