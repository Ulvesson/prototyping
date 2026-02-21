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

    // Calculate milestone bonus (2x per 10 levels, 10x per 50 levels)
    double getMilestoneMultiplier() const {
        int milestones10 = owned / 10;
        int milestones50 = owned / 50;
        
        // 2x bonus per 10 levels
        double bonus10 = std::pow(2.0, milestones10);
        
        // Additional 10x bonus per 50 levels
        double bonus50 = std::pow(10.0, milestones50);
        
        return bonus10 * bonus50;
    }

    // Get effective tax per second with milestone bonus
    double getEffectiveTaxPerSecond(double prestigeBonus, double levelBonus) const {
        //  //upgrade.baseTaxPerSecond * upgrade.owned * upgrade.getMilestoneMultiplier();
		double tax = baseTaxPerSecond * owned * getMilestoneMultiplier();
        tax += tax * levelBonus;
		return tax += tax * prestigeBonus;  // Apply prestige bonus to tax per second
    }

    // Get next milestone info
    int getLevelsToNextMilestone() const {
        return 10 - (owned % 10);
    }

    int getNextMilestoneLevel() const {
        return ((owned / 10) + 1) * 10;
    }
    
    // Get next major milestone (50 level) info
    int getLevelsToNextMajorMilestone() const {
        return 50 - (owned % 50);
    }
    
    int getNextMajorMilestoneLevel() const {
        return ((owned / 50) + 1) * 50;
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

    // Calculate milestone bonus (2x per 10 levels, 10x per 50 levels)
    double getMilestoneMultiplier() const {
        int milestones10 = owned / 10;
        int milestones50 = owned / 50;
        
        // 2x bonus per 10 levels
        double bonus10 = std::pow(2.0, milestones10);
        
        // Additional 10x bonus per 50 levels
        double bonus50 = std::pow(10.0, milestones50);
        
        return bonus10 * bonus50;
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
    
    // Get next major milestone (50 level) info
    int getLevelsToNextMajorMilestone() const {
        return 50 - (owned % 50);
    }
    
    int getNextMajorMilestoneLevel() const {
        return ((owned / 50) + 1) * 50;
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
    
    // Auto-upgrade functions
    void autoUpgradePassiveIncomes();
    void toggleAutoUpgrade(int upgradeIndex);
    
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
    int playerLevel = 1;  // Start at level 1
    uint64_t currentXP = 0;
    double lastTaxesForXP = 0.0;
    
    // Prestige/Ascension system
    int prestigeStars = 0;         // Permanent stars from ascensions
    int totalAscensions = 0;       // Track how many times ascended
    bool ascensionConfirmationPending = false;
    double ascensionConfirmationTimer = 0.0;
    
    // Time tracking
    double totalPlayTime = 0.0;           // Total time played across all sessions (in seconds)
    double timeSinceLastAscension = 0.0;  // Time since last ascension (in seconds)
    
    static constexpr double PRESTIGE_BONUS_PER_STAR = 0.50;

    // Upgrades
    std::vector<Upgrade> upgrades;
    std::vector<ClickUpgrade> clickUpgrades;
    std::vector<bool> autoUpgradeEnabled;  // Track which upgrades have auto-upgrade enabled

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
    static constexpr double TAXES_PER_XP = 50.0;
    static constexpr double BONUS_PER_LEVEL = 0.005;
	static constexpr double XP_BASE = 100.0;

#ifdef _DEBUG
	static constexpr double DEBUG_MANUAL_TAX_PER_CLICK = 10000000000.0;
	static constexpr double DEBUG_BONUS_PER_LEVEL = BONUS_PER_LEVEL;
    static constexpr double DEBUG_START_MONEY = 10000000.0;
    static constexpr int DEBUG_START_LEVEL = 1;  // Start at level 1
#endif // DEBUG

};