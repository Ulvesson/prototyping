#pragma once

#include <string>
#include <vector>
#include <chrono>

class Game {
public:
    struct Upgrade {
        std::string name;
        std::string description;
        double cost;
        double incomePerSecond;
        int owned;
        double costMultiplier;
    };

    Game();
    void update();
    void click();
    bool purchaseUpgrade(size_t index);
    void displayStatus() const;
    void displayUpgrades() const;
    
    bool isRunning() const { return running_; }
    void quit() { running_ = false; }

private:
    double resources_;
    double clickValue_;
    double incomePerSecond_;
    std::chrono::steady_clock::time_point lastUpdate_;
    std::vector<Upgrade> upgrades_;
    bool running_;

    void calculateIncomePerSecond();
    double getUpgradeCost(const Upgrade& upgrade) const;
};