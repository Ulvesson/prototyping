#include "Game.h"
#include <iostream>
#include <iomanip>
#include <cmath>

Game::Game()
    : resources_(0.0)
    , clickValue_(1.0)
    , incomePerSecond_(0.0)
    , lastUpdate_(std::chrono::steady_clock::now())
    , running_(true)
{
    // Initialize upgrades
    upgrades_ = {
        {"Auto-Clicker", "Generates 0.5 resources/sec", 10.0, 0.5, 0, 1.15},
        {"Mining Rig", "Generates 5 resources/sec", 100.0, 5.0, 0, 1.15},
        {"Factory", "Generates 50 resources/sec", 1000.0, 50.0, 0, 1.15},
        {"Corporation", "Generates 500 resources/sec", 10000.0, 500.0, 0, 1.15},
        {"Quantum Generator", "Generates 5000 resources/sec", 100000.0, 5000.0, 0, 1.15}
    };
}

void Game::update() {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastUpdate_);
    double deltaTime = duration.count() / 1000.0;
    
    if (deltaTime > 0.0) {
        resources_ += incomePerSecond_ * deltaTime;
        lastUpdate_ = now;
    }
}

void Game::click() {
    resources_ += clickValue_;
    std::cout << "+" << clickValue_ << " resources! (Total: " 
              << std::fixed << std::setprecision(2) << resources_ << ")\n";
}

bool Game::purchaseUpgrade(size_t index) {
    if (index >= upgrades_.size()) {
        return false;
    }
    
    auto& upgrade = upgrades_[index];
    double cost = getUpgradeCost(upgrade);
    
    if (resources_ >= cost) {
        resources_ -= cost;
        upgrade.owned++;
        upgrade.cost = cost * upgrade.costMultiplier;
        calculateIncomePerSecond();
        
        std::cout << "\n✓ Purchased " << upgrade.name << "! (Now own: " 
                  << upgrade.owned << ")\n";
        return true;
    }
    
    std::cout << "\n✗ Not enough resources! Need " 
              << std::fixed << std::setprecision(2) << cost << "\n";
    return false;
}

void Game::displayStatus() const {
    std::cout << "\n═══════════════════════════════════════════\n";
    std::cout << "  IDLE GAME - Status\n";
    std::cout << "═══════════════════════════════════════════\n";
    std::cout << "Resources: " << std::fixed << std::setprecision(2) << resources_ << "\n";
    std::cout << "Income/sec: " << std::fixed << std::setprecision(2) << incomePerSecond_ << "\n";
    std::cout << "Click value: " << std::fixed << std::setprecision(2) << clickValue_ << "\n";
    std::cout << "═══════════════════════════════════════════\n\n";
}

void Game::displayUpgrades() const {
    std::cout << "Available Upgrades:\n";
    std::cout << "───────────────────────────────────────────\n";
    
    for (size_t i = 0; i < upgrades_.size(); ++i) {
        const auto& upgrade = upgrades_[i];
        double cost = getUpgradeCost(upgrade);
        
        std::cout << "[" << (i + 1) << "] " << upgrade.name 
                  << " (Owned: " << upgrade.owned << ")\n";
        std::cout << "    " << upgrade.description << "\n";
        std::cout << "    Cost: " << std::fixed << std::setprecision(2) 
                  << cost << " resources\n\n";
    }
}

void Game::calculateIncomePerSecond() {
    incomePerSecond_ = 0.0;
    for (const auto& upgrade : upgrades_) {
        incomePerSecond_ += upgrade.incomePerSecond * upgrade.owned;
    }
}

double Game::getUpgradeCost(const Upgrade& upgrade) const {
    if (upgrade.owned == 0) {
        return upgrade.cost;
    }
    return upgrade.cost * std::pow(upgrade.costMultiplier, upgrade.owned);
}