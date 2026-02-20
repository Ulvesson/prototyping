#pragma once
#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>
#include <string>
#include <cstdint>
#include <cmath>

struct Upgrade {
    std::string name;
    std::string description;
    double baseCost;
    double baseTaxPerSecond;
    int owned;
    double costMultiplier;

    uint64_t getCurrentCost() const {  // Changed from double to uint64_t
        return static_cast<uint64_t>(baseCost * std::pow(costMultiplier, owned));
    }

    // Calculate milestone bonus (2x per 10 levels)
    double getMilestoneMultiplier() const {
        int milestones = owned / 10;
        return std::pow(2.0, milestones);
    }

    // Get effective tax per second with milestone bonus
    double getEffectiveTaxPerSecond() const {
        return baseTaxPerSecond * getMilestoneMultiplier();
    }

    // Get next milestone info
    int getLevelsToNextMilestone() const {
        return 10 - (owned % 10);
    }

    int getNextMilestoneLevel() const {
        return ((owned / 10) + 1) * 10;
    }
};

struct ClickUpgrade {
    std::string name;
    std::string description;
    double baseCost;
    double baseClickValueIncrease;
    int owned;
    double costMultiplier;

    uint64_t getCurrentCost() const {  // Changed from double to uint64_t
        return static_cast<uint64_t>(baseCost * std::pow(costMultiplier, owned));
    }

    // Calculate milestone bonus (2x per 10 levels)
    double getMilestoneMultiplier() const {
        int milestones = owned / 10;
        return std::pow(2.0, milestones);
    }

    // Get effective click value with milestone bonus
    double getEffectiveClickValue() const {
        return baseClickValueIncrease * getMilestoneMultiplier();
    }

    // Get next milestone info
    int getLevelsToNextMilestone() const {
        return 10 - (owned % 10);
    }

    int getNextMilestoneLevel() const {
        return ((owned / 10) + 1) * 10;
    }
};

class Game {
public:
    Game();
    ~Game();

    bool initialize();
    void run();
    void cleanup();

private:
    void handleEvents();
    void handleMouseClick(int x, int y);
    void tryPurchaseUpgrade(int upgradeIndex);
    void tryPurchaseClickUpgrade(int upgradeIndex);
    void tryPurchaseUpgradeMax(int upgradeIndex);        // New: Buy max upgrades
    void tryPurchaseClickUpgradeMax(int upgradeIndex);   // New: Buy max click upgrades
    void update(double deltaTime);
    void render();
    void renderText(const std::string& text, int x, int y, SDL_Color color);
    void renderButton(const std::string& text, int x, int y, int width, int height, bool canAfford);
    void renderLevelBar();
    void renderStars();
    
    // Save/Load functions
    void saveGame(const std::string& filename = "savegame.txt");
    bool loadGame(const std::string& filename = "savegame.txt");
    void showSaveNotification(const std::string& message);
    void resetGame();
    void ascendGame();  // New: Ascension/Prestige system
    
    // Helper to recalculate stats
    void recalculateTaxesPerSecond();
    void recalculateClickValue();
    int calculateMaxPurchasable(double baseCost, double costMultiplier, int currentOwned, double availableMoney) const;
    
    // Level system
    void updateExperience();
    uint64_t getXPForNextLevel() const;
    double getXPProgress() const;
    double getLevelBonus() const;
    double getPrestigeBonus() const;  // New: Bonus from stars
    double getTimeToNextLevel() const;
    std::string formatTime(double seconds) const;
    std::string formatNumber(double value) const;
    std::string formatNumber(uint64_t value) const;
    int getStarCount() const;
    bool canAscend() const;  // New: Check if ascension is available

    // SDL components
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    TTF_Font* font = nullptr;
    TTF_Font* titleFont = nullptr;
    TTF_Font* starFont = nullptr;

    // Game state
    bool running = false;
    double totalTaxes = 0.0;
    double taxesPerSecond = 0.0;
    double lifetimeTaxes = 0.0;
    double manualTaxPerClick = 1.0;
    
        // Level system
    int playerLevel = 0;  // Changed from 1 to 0
    uint64_t currentXP = 0;
    double lastTaxesForXP = 0.0;
    
    // Prestige/Ascension system
    int prestigeStars = 0;         // Permanent stars from ascensions
    int totalAscensions = 0;       // Track how many times ascended
    bool ascensionConfirmationPending = false;
    double ascensionConfirmationTimer = 0.0;
    
    static constexpr double PRESTIGE_BONUS_PER_STAR = 0.50;  // 50% per star

    // Upgrades
    std::vector<Upgrade> upgrades;
    std::vector<ClickUpgrade> clickUpgrades;

    // Save notification
    std::string notificationMessage;
    double notificationTimer = 0.0;
    
    // Reset confirmation
    bool resetConfirmationPending = false;
    double resetConfirmationTimer = 0.0;

    // Constants
    static constexpr int WINDOW_WIDTH = 1200;
    static constexpr int WINDOW_HEIGHT = 800;
    static constexpr int UPGRADE_Y_START = 180;
    static constexpr int UPGRADE_HEIGHT = 60;
    static constexpr int UPGRADE_SPACING = 10;
    static constexpr int CLICK_UPGRADE_X_START = 800;
    static constexpr double TAXES_PER_XP = 1.0;
    static constexpr double BONUS_PER_LEVEL = 0.02;

#ifdef _DEBUG
	static constexpr double DEBUG_MANUAL_TAX_PER_CLICK = 1000000000.0;
	static constexpr double DEBUG_BONUS_PER_LEVEL = 10.0; // 1000% bonus per level for testing
    static constexpr double DEBUG_START_MONEY = 10000.0;
    static constexpr int DEBUG_START_LEVEL = 0;
#endif // DEBUG

};