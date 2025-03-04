#include "cfr_network.hpp"
#include <fstream>
#include <iostream>

CFRNet::CFRNet(int input_size, int num_actions) : num_actions(num_actions) {
    // Create a simple feedforward network
    model = torch::nn::Sequential(
        torch::nn::Linear(input_size, 128),
        torch::nn::ReLU(),
        torch::nn::Linear(128, 64),
        torch::nn::ReLU(),
        torch::nn::Linear(64, num_actions)
    );

    // Create optimizer
    optimizer = new torch::optim::Adam(model->parameters(), 0.001);
}

std::vector<float> CFRNet::predict(const std::vector<float>& features) {
    // Convert input to tensor
    torch::Tensor input = torch::tensor(features).reshape({ 1, -1 });

    // Set to evaluation mode
    model->eval();

    // Forward pass (with no gradient computation)
    torch::NoGradGuard no_grad;
    torch::Tensor output = model->forward(input);

    // Convert output to vector
    std::vector<float> result(output.data_ptr<float>(),
                            output.data_ptr<float>() + output.numel());

    // Set back to training mode
    model->train();

    return result;
}

void CFRNet::train(const std::vector<std::pair<std::string, std::vector<float>>>& batch) {
    // Prepare batch data
    std::vector<std::vector<float>> features;
    std::vector<std::vector<float>> targets;

    for (const auto& sample : batch) {
        features.push_back(parseFeatures(sample.first));
        targets.push_back(sample.second);
    }

    // Convert to tensors
    torch::Tensor x = torch::from_blob(features.data(), { static_cast<long>(features.size()),
                                      static_cast<long>(features[0].size()) });
    torch::Tensor y = torch::from_blob(targets.data(), { static_cast<long>(targets.size()),
                                      static_cast<long>(targets[0].size()) });

    // Training loop
    for (int epoch = 0; epoch < 10; epoch++) {
        // Forward pass
        torch::Tensor output = model->forward(x);

        // Compute MSE loss
        torch::Tensor loss = torch::mse_loss(output, y);

        // Backward pass
        optimizer->zero_grad();
        loss.backward();
        optimizer->step();

        std::cout << "Epoch " << epoch << ", Loss: " << loss.item<float>() << std::endl;
    }
}

void CFRNet::save(const std::string& path) {
    torch::save(model, path);
}

void CFRNet::load(const std::string& path) {
    torch::load(model, path);
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