#include "cfr_network.hpp"
#include <fstream>
#include <iostream>

CFRNet::CFRNet(int input_size, int num_actions) : num_actions(num_actions) {
    // Create a simple feedforward network
    model = std::make_shared<mytorch::nn::Sequential>();

    // Add layers
    model->add_module(std::make_shared<mytorch::Linear>(input_size, 128));
    model->add_module(std::make_shared<mytorch::nn::ReLU>());
    model->add_module(std::make_shared<mytorch::Linear>(128, 64));
    model->add_module(std::make_shared<mytorch::nn::ReLU>());
    model->add_module(std::make_shared<mytorch::Linear>(64, num_actions));
}

std::vector<float> CFRNet::predict(const std::vector<float>& features) {
    // Convert input to tensor
    mytorch::Tensor input(features, { 1, static_cast<size_t>(features.size()) });

    // Forward pass
    mytorch::Tensor output = model->forward(input);

    // Convert output to vector
    return output.to_vector();
}

void CFRNet::train(const std::vector<std::pair<std::string, std::vector<float>>>& batch) {
    // Create optimizer
    std::vector<std::reference_wrapper<mytorch::Module>> modules = { *model };
    mytorch::Adam optimizer(modules, 0.001f);

    // Prepare batch data
    std::vector<std::vector<float>> features;
    std::vector<std::vector<float>> targets;

    for (const auto& sample : batch) {
        // Parse features and targets from sample
        std::vector<float> feature = parseFeatures(sample.first);
        std::vector<float> target = sample.second;

        features.push_back(feature);
        targets.push_back(target);
    }

    // Training loop
    for (int epoch = 0; epoch < 10; epoch++) {
        float total_loss = 0.0f;

        for (size_t i = 0; i < features.size(); ++i) {
            // Convert to tensors
            mytorch::Tensor x(features[i], { 1, features[i].size() });
            mytorch::Tensor y(targets[i], { 1, targets[i].size() });

            // Forward pass
            mytorch::Tensor output = model->forward(x);

            // Compute loss (MSE)
            mytorch::Tensor loss = mytorch::Tensor::mse_loss(output, y);
            total_loss += loss.item();

            // Backward pass
            optimizer.zero_grad();
            loss.backward();
            optimizer.step();
        }

        std::cout << "Epoch " << epoch << ", Loss: " << total_loss / features.size() << std::endl;
    }
}

void CFRNet::save(const std::string& path) {
    // In a real implementation, this would serialize the model parameters
    std::cout << "Model saved to " << path << " (not implemented)" << std::endl;
}

void CFRNet::load(const std::string& path) {
    // In a real implementation, this would deserialize the model parameters
    std::cout << "Model loaded from " << path << " (not implemented)" << std::endl;
}

std::vector<float> CFRNet::parseFeatures(const std::string& info_state) {
    // Simple implementation - in a real system this would parse the info state
    // For now, just return a dummy vector
    return std::vector<float>(64, 0.5f);
}

// ValueNet implementation
ValueNet::ValueNet(int num_players, int input_size, int num_actions) {
    networks.resize(num_players);
    for (int i = 0; i < num_players; ++i) {
        networks[i] = std::make_shared<CFRNet>(input_size, num_actions);
    }
}

std::vector<float> ValueNet::predict(int player_id, const std::vector<float>& features) {
    if (player_id < 0 || player_id >= static_cast<int>(networks.size())) {
        throw std::runtime_error("Invalid player ID");
    }
    return networks[player_id]->predict(features);
}

void ValueNet::train(int player_id, const std::vector<std::pair<std::string, std::vector<float>>>& batch) {
    if (player_id < 0 || player_id >= static_cast<int>(networks.size())) {
        throw std::runtime_error("Invalid player ID");
    }
    networks[player_id]->train(batch);
}

// StrategyNet implementation
StrategyNet::StrategyNet(int input_size, int num_actions) {
    network = std::make_shared<CFRNet>(input_size, num_actions);
}

std::vector<float> StrategyNet::predict(const std::vector<float>& features) {
    return network->predict(features);
}

void StrategyNet::train(const std::vector<std::pair<std::string, std::vector<float>>>& batch) {
    network->train(batch);
}