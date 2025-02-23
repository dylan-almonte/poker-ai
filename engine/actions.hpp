#pragma once

#include <string>
#include <vector>
#include <stdexcept>

class Action {
public:
    Action() = default;
    virtual ~Action() = default;
    virtual std::string toString() const = 0;
};

class Call : public Action {
public:
    std::string toString() const override {
        return "call";
    }
};

class Fold : public Action {
public:
    std::string toString() const override {
        return "fold";
    }
};

class Raise : public Action {
public:
    void setAmount(int amount) {
        amount_ = amount;
    }

    std::string toString() const override {
        return "raise";
    }

private:
    int amount_;
};

class AbstractedRaise : public Action {
public:
    explicit AbstractedRaise(const std::vector<int>& allowed_amounts) 
        : amounts_(allowed_amounts) {}

    void setAmount(int amount) {
        // Check if amount is valid
        bool valid = false;
        for (int allowed : amounts_) {
            if (amount == allowed) {
                valid = true;
                break;
            }
        }
        
        if (!valid) {
            throw std::runtime_error(
                "Specified amount is not valid for this action abstraction");
        }
        amount_ = amount;
    }

    const std::vector<int>& getAllowedAmounts() const {
        return amounts_;
    }

    std::string toString() const override {
        return "raise " + std::to_string(amount_);
    }

private:
    std::vector<int> amounts_;
    int amount_;
};

// Default amounts that can be used for AbstractedRaise
const std::vector<int> DUMMY_AMOUNTS = {10, 100, 500, 1000, 5000, 10000}; 