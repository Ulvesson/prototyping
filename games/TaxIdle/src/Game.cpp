#include "Game.h"
#include <format>
#include <cmath>
#include <fstream>
#include <sstream>

Game::Game() {
    // Initialize passive income upgrades
    upgrades = {
        {"Tax Collector", "A comrade to collect taxes manually", 15.0, 0.1, 0, 1.15},
        {"Tax Office", "A small office for tax collection", 100.0, 1.0, 0, 1.15},
        {"Ministry of Finance", "Manages regional tax collection", 1100.0, 8.0, 0, 1.15},
        {"Central Planning Committee", "Optimizes tax collection efficiency", 12000.0, 47.0, 0, 1.15},
        {"State Bank", "Controls all financial resources", 130000.0, 260.0, 0, 1.15},
        {"Propaganda Ministry", "Convinces citizens to pay more taxes", 1400000.0, 1400.0, 0, 1.15},
        {"Supreme Tax Authority", "Ultimate control over all wealth", 20000000.0, 7800.0, 0, 1.15},
    };

    // Initialize click value upgrades
    clickUpgrades = {
        {"Better Pen", "Write taxes faster", 10.0, 1.0, 0, 1.15},
        {"Tax Forms", "Efficient paperwork", 100.0, 5.0, 0, 1.20},
        {"Calculator", "Speed up calculations", 500.0, 15.0, 0, 1.25},
        {"Tax Software", "Automate simple tasks", 2500.0, 50.0, 0, 1.30},
        {"Expert Training", "Become a tax expert", 10000.0, 150.0, 0, 1.35},
        {"Elite Status", "Master tax collector", 50000.0, 500.0, 0, 1.40},
    };

#ifdef _DEBUG
    // Debug mode: Start with higher click value for testing
    manualTaxPerClick = DEBUG_MANUAL_TAX_PER_CLICK;
    SDL_Log("DEBUG MODE: Starting with 1000 click value");
#endif
}

Game::~Game() {
    // Auto-save on exit
    saveGame();
    cleanup();
}

bool Game::initialize() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        return false;
    }

    if (TTF_Init() < 0) {
        SDL_Log("TTF initialization failed: %s", TTF_GetError());
        return false;
    }

    window = SDL_CreateWindow(
        "Tax Idle Game - For The People!",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );

    if (!window) {
        SDL_Log("Window creation failed: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        SDL_Log("Renderer creation failed: %s", SDL_GetError());
        return false;
    }

    font = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 16);
    titleFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 24);
    starFont = TTF_OpenFont("C:/Windows/Fonts/seguisym.ttf", 36);  // Segoe UI Symbol has good star
    
    if (!font || !titleFont) {
        SDL_Log("Warning: Could not load font: %s", TTF_GetError());
    }
    
    if (!starFont) {
        SDL_Log("Warning: Could not load star font, falling back to arial");
        starFont = TTF_OpenFont("C:/Windows/Fonts/arial.ttf", 36);
    }

    running = true;
    
    // Try to load saved game
    if (loadGame()) {
        showSaveNotification("Game loaded successfully!");
    }
    
    return true;
}

void Game::run() {
    Uint64 lastTime = SDL_GetPerformanceCounter();
    const double freq = static_cast<double>(SDL_GetPerformanceFrequency());

    while (running) {
        Uint64 currentTime = SDL_GetPerformanceCounter();
        double deltaTime = static_cast<double>(currentTime - lastTime) / freq;
        lastTime = currentTime;

        handleEvents();
        update(deltaTime);
        render();
    }
}

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (event.button.button == SDL_BUTTON_LEFT) {
                    handleMouseClick(event.button.x, event.button.y);
                }
                break;
            case SDL_KEYDOWN:
                if (event.key.keysym.sym == SDLK_s) {
                    saveGame();
                    showSaveNotification("Game saved!");
                }
                else if (event.key.keysym.sym == SDLK_l) {
                    if (loadGame()) {
                        showSaveNotification("Game loaded!");
                    } else {
                        showSaveNotification("No save file found!");
                    }
                }
                else if (event.key.keysym.sym == SDLK_r) {
                    if (resetConfirmationPending) {
                        resetGame();
                    } else {
                        resetConfirmationPending = true;
                        resetConfirmationTimer = 5.0;
                        showSaveNotification("Press 'R' again within 5 seconds to RESET ALL PROGRESS!");
                    }
                }
                else if (event.key.keysym.sym == SDLK_a) {  // New: Ascend with 'A' key
                    if (!canAscend()) {
                        showSaveNotification("Reach Level 100 to unlock Ascension!");
                    } else if (ascensionConfirmationPending) {
                        ascendGame();
                    } else {
                        ascensionConfirmationPending = true;
                        ascensionConfirmationTimer = 5.0;
                        int starsToGain = playerLevel / 100;
                        std::string msg = std::format("Press 'A' again to ASCEND and gain {} Prestige Star(s)!", starsToGain);
                        showSaveNotification(msg);
                    }
                }
                break;
        }
    }
}

void Game::handleMouseClick(int x, int y) {
    // Check if clicking the main tax button (red square area)
    if (x >= 50 && x <= 250 && y >= 180 && y <= 380) {
        totalTaxes += manualTaxPerClick;
        lifetimeTaxes += manualTaxPerClick;
    }

    // Check passive income upgrade buttons (left side)
    for (size_t i = 0; i < upgrades.size(); ++i) {
        int buttonY = UPGRADE_Y_START + static_cast<int>(i) * (UPGRADE_HEIGHT + UPGRADE_SPACING);
        
        // Check MAX button (right side of upgrade button)
        if (x >= 720 && x <= 770 && y >= buttonY && y <= buttonY + UPGRADE_HEIGHT) {
            tryPurchaseUpgradeMax(static_cast<int>(i));
        }
        // Check regular upgrade button
        else if (x >= 320 && x <= 710 && y >= buttonY && y <= buttonY + UPGRADE_HEIGHT) {
            tryPurchaseUpgrade(static_cast<int>(i));
        }
    }

    // Check click value upgrade buttons (right side)
    for (size_t i = 0; i < clickUpgrades.size(); ++i) {
        int buttonY = UPGRADE_Y_START + static_cast<int>(i) * (UPGRADE_HEIGHT + UPGRADE_SPACING);
        
        // Check MAX button (right side of upgrade button)
        if (x >= CLICK_UPGRADE_X_START + 330 && x <= CLICK_UPGRADE_X_START + 380 && 
            y >= buttonY && y <= buttonY + UPGRADE_HEIGHT) {
            tryPurchaseClickUpgradeMax(static_cast<int>(i));
        }
        // Check regular upgrade button
        else if (x >= CLICK_UPGRADE_X_START && x <= CLICK_UPGRADE_X_START + 320 && 
            y >= buttonY && y <= buttonY + UPGRADE_HEIGHT) {
            tryPurchaseClickUpgrade(static_cast<int>(i));
        }
    }
}

void Game::tryPurchaseUpgrade(int upgradeIndex) {
    if (upgradeIndex < 0 || upgradeIndex >= upgrades.size()) return;

    Upgrade& upgrade = upgrades[upgradeIndex];
    uint64_t cost = upgrade.getCurrentCost();  // Changed to uint64_t

    if (totalTaxes >= cost) {
        int oldLevel = upgrade.owned;
        totalTaxes -= cost;
        upgrade.owned++;
        
        // Recalculate total taxes per second with milestone bonuses
        recalculateTaxesPerSecond();
        
        // Check if milestone reached
        if (oldLevel / 10 != upgrade.owned / 10) {
            std::string msg = std::format("MILESTONE! {} Lv{} - {}x Bonus!", 
                upgrade.name, upgrade.owned, static_cast<int>(upgrade.getMilestoneMultiplier()));
            showSaveNotification(msg);
        }
    }
}

void Game::tryPurchaseClickUpgrade(int upgradeIndex) {
    if (upgradeIndex < 0 || upgradeIndex >= clickUpgrades.size()) return;

    ClickUpgrade& upgrade = clickUpgrades[upgradeIndex];
    uint64_t cost = upgrade.getCurrentCost();  // Changed to uint64_t

    if (totalTaxes >= cost) {
        int oldLevel = upgrade.owned;
        totalTaxes -= cost;
        upgrade.owned++;
        
        // Recalculate click value with milestone bonuses
        recalculateClickValue();
        
        // Check if milestone reached
        if (oldLevel / 10 != upgrade.owned / 10) {
            std::string msg = std::format("MILESTONE! {} Lv{} - {}x Click Power!", 
                upgrade.name, upgrade.owned, static_cast<int>(upgrade.getMilestoneMultiplier()));
            showSaveNotification(msg);
        }
    }
}

double Game::getLevelBonus() const {
#ifdef _DEBUG
    return 1.0 + (playerLevel * DEBUG_BONUS_PER_LEVEL);
#else
    return 1.0 + (playerLevel * BONUS_PER_LEVEL);
#endif
}

void Game::recalculateTaxesPerSecond() {
    taxesPerSecond = 0.0;
    for (const auto& upgrade : upgrades) {
        if (upgrade.owned > 0) {
            taxesPerSecond += upgrade.baseTaxPerSecond * upgrade.owned * upgrade.getMilestoneMultiplier();
        }
    }
    // Apply global level bonus
    taxesPerSecond *= getLevelBonus();

    // Apply prestige bonus
    taxesPerSecond *= getPrestigeBonus();
}

void Game::recalculateClickValue() {
#ifdef _DEBUG
	manualTaxPerClick = DEBUG_MANUAL_TAX_PER_CLICK; // Start with debug value
#else
    manualTaxPerClick = 1.0; // Base click value
#endif
    for (const auto& upgrade : clickUpgrades) {
        if (upgrade.owned > 0) {
            manualTaxPerClick += upgrade.baseClickValueIncrease * upgrade.owned * upgrade.getMilestoneMultiplier();
        }
    }
    // Apply global level bonus
    manualTaxPerClick *= getLevelBonus();

    // Apply prestige bonus
    manualTaxPerClick *= getPrestigeBonus();
}

uint64_t Game::getXPForNextLevel() const {
    // Special case for level 0 -> 1
    if (playerLevel == 0) {
        return 100;  // First level requires 100 XP
    }

    // XP required = 10 * level * (1.1^level) - Compound scaling
    return static_cast<uint64_t>(10.0 * playerLevel * std::pow(1.1, playerLevel));
}

double Game::getXPProgress() const {
    uint64_t xpNeeded = getXPForNextLevel();
    if (xpNeeded == 0) return 0.0;
    return static_cast<double>(currentXP) / static_cast<double>(xpNeeded);
}

void Game::updateExperience() {
    // Calculate how much lifetime taxes have increased
    double taxesSinceLastXP = lifetimeTaxes - lastTaxesForXP;
    
    // Award XP (1 XP per 10 taxes)
    uint64_t xpGained = static_cast<uint64_t>(taxesSinceLastXP / TAXES_PER_XP);
    
    if (xpGained > 0) {
        currentXP += xpGained;
        lastTaxesForXP += xpGained * TAXES_PER_XP;
        
        // Check for level up
        uint64_t xpNeeded = getXPForNextLevel();
        while (currentXP >= xpNeeded) {
            currentXP -= xpNeeded;
            playerLevel++;
            
            // Recalculate everything with new level bonus
            recalculateTaxesPerSecond();
            recalculateClickValue();
            
            int bonusPercent = static_cast<int>((getLevelBonus() - 1.0) * 100);
            std::string msg = std::format("LEVEL UP! Lv{} - Global Tax Bonus: +{}%", 
                playerLevel, bonusPercent);
            showSaveNotification(msg);
            
            xpNeeded = getXPForNextLevel();
        }
    }
}

double Game::getTimeToNextLevel() const {
    if (taxesPerSecond <= 0.0) {
        return -1.0; // Infinite time (no passive income)
    }
    
    uint64_t xpNeeded = getXPForNextLevel();
    if (currentXP >= xpNeeded) {
        return 0.0;
    }
    
    uint64_t xpRemaining = xpNeeded - currentXP;
    
    // Calculate taxes needed for remaining XP
    double taxesNeeded = static_cast<double>(xpRemaining) * TAXES_PER_XP;
    
    // Time = taxes needed / taxes per second
    return taxesNeeded / taxesPerSecond;
}

void Game::update(double deltaTime) {
    // Generate passive income
    totalTaxes += taxesPerSecond * deltaTime;
    lifetimeTaxes += taxesPerSecond * deltaTime;
    
    // Update experience
    updateExperience();
    
    // Update notification timer
    if (notificationTimer > 0.0) {
        notificationTimer -= deltaTime;
    }
    
    // Update reset confirmation timer
    if (resetConfirmationPending) {
        resetConfirmationTimer -= deltaTime;
        if (resetConfirmationTimer <= 0.0) {
            resetConfirmationPending = false;
            showSaveNotification("Reset cancelled.");
        }
    }

    // Update ascension confirmation timer
    if (ascensionConfirmationPending) {
        ascensionConfirmationTimer -= deltaTime;
        if (ascensionConfirmationTimer <= 0.0) {
            ascensionConfirmationPending = false;
            showSaveNotification("Ascension cancelled.");
        }
    }
}

void Game::saveGame(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        SDL_Log("Failed to save game to %s", filename.c_str());
        return;
    }

    file << "VERSION=2\n";  // Increment version for prestige support

    // Save core game state
    file << "TOTAL_TAXES=" << totalTaxes << "\n";
    file << "LIFETIME_TAXES=" << lifetimeTaxes << "\n";
    file << "MANUAL_TAX_PER_CLICK=" << manualTaxPerClick << "\n";
    file << "TAXES_PER_SECOND=" << taxesPerSecond << "\n";

    // Save level system
    file << "PLAYER_LEVEL=" << playerLevel << "\n";
    file << "CURRENT_XP=" << currentXP << "\n";
    file << "LAST_TAXES_FOR_XP=" << lastTaxesForXP << "\n";

    // Save prestige system
    file << "PRESTIGE_STARS=" << prestigeStars << "\n";
    file << "TOTAL_ASCENSIONS=" << totalAscensions << "\n";

    // Save upgrades...
    file << "UPGRADE_COUNT=" << upgrades.size() << "\n";
    for (size_t i = 0; i < upgrades.size(); ++i) {
        file << "UPGRADE_" << i << "=" << upgrades[i].owned << "\n";
    }

    file << "CLICK_UPGRADE_COUNT=" << clickUpgrades.size() << "\n";
    for (size_t i = 0; i < clickUpgrades.size(); ++i) {
        file << "CLICK_UPGRADE_" << i << "=" << clickUpgrades[i].owned << "\n";
    }

    file.close();
    SDL_Log("Game saved to %s", filename.c_str());
}

bool Game::loadGame(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        SDL_Log("No save file found at %s", filename.c_str());
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);

        if (key == "VERSION") {
            // Version tracking
        }
        else if (key == "TOTAL_TAXES") {
            totalTaxes = std::stod(value);
        }
        else if (key == "LIFETIME_TAXES") {
            lifetimeTaxes = std::stod(value);
        }
        else if (key == "PLAYER_LEVEL") {
            playerLevel = std::stoi(value);
        }
        else if (key == "CURRENT_XP") {
            currentXP = std::stoull(value);
        }
        else if (key == "LAST_TAXES_FOR_XP") {
            lastTaxesForXP = std::stod(value);
        }
        else if (key == "PRESTIGE_STARS") {
            prestigeStars = std::stoi(value);
        }
        else if (key == "TOTAL_ASCENSIONS") {
            totalAscensions = std::stoi(value);
        }
        else if (key.starts_with("UPGRADE_") && !key.starts_with("UPGRADE_COUNT")) {
            size_t index = std::stoull(key.substr(8));
            if (index < upgrades.size()) {
                upgrades[index].owned = std::stoi(value);
            }
        }
        else if (key.starts_with("CLICK_UPGRADE_") && !key.starts_with("CLICK_UPGRADE_COUNT")) {
            size_t index = std::stoull(key.substr(14));
            if (index < clickUpgrades.size()) {
                clickUpgrades[index].owned = std::stoi(value);
            }
        }
    }

    // Recalculate with bonuses
    recalculateTaxesPerSecond();
    recalculateClickValue();

    file.close();
    SDL_Log("Game loaded from %s", filename.c_str());
    return true;
}

void Game::showSaveNotification(const std::string& message) {
    notificationMessage = message;
    notificationTimer = 3.0; // Show for 3 seconds
}

void Game::renderLevelBar() {
    const int barX = 50;
    const int barY = 730;
    const int barWidth = 1100;
    const int barHeight = 30;

    // Background bar
    SDL_Rect bgRect = { barX, barY, barWidth, barHeight };
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_RenderFillRect(renderer, &bgRect);

    // XP progress bar
    double progress = getXPProgress();
    int fillWidth = static_cast<int>(barWidth * progress);
    SDL_Rect fillRect = { barX, barY, fillWidth, barHeight };

    // Gradient effect (darker to lighter red)
    SDL_SetRenderDrawColor(renderer, 200, 50, 50, 255);
    SDL_RenderFillRect(renderer, &fillRect);

    // Border
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_RenderDrawRect(renderer, &bgRect);

    // Level text with bonus and time to next level
    int bonusPercent = static_cast<int>((getLevelBonus() - 1.0) * 100);
    double timeToNext = getTimeToNextLevel();
    std::string timeStr = formatTime(timeToNext);

    std::string levelText;
    if (playerLevel == 0) {
        // At level 0, don't show bonus percentage (it's 0%)
        levelText = std::format("Level {} | XP: {}/{} | Next: {}",
            playerLevel, formatNumber(currentXP), formatNumber(getXPForNextLevel()), timeStr);
    }
    else {
        levelText = std::format("Level {} (+{}% Bonus) | XP: {}/{} | Next: {}",
            playerLevel, bonusPercent, formatNumber(currentXP), formatNumber(getXPForNextLevel()), timeStr);
    }
    renderText(levelText, barX + 10, barY + 7, { 255, 255, 255, 255 });

    // Progress percentage
    std::string progressText = std::format("{:.1f}%", progress * 100.0);
    renderText(progressText, barX + barWidth - 80, barY + 7, { 255, 215, 0, 255 });
}

int Game::getStarCount() const {
    // 1 star per 100 levels, maximum 5 stars
    int stars = playerLevel / 100;
    return std::min(stars, 5);
}

void Game::renderStars() {
    int starCount = getStarCount();
    if (starCount == 0) return;
    
    const int startX = WINDOW_WIDTH - 280;  // Upper right area
    const int startY = 0;
    const int spacing = 30;
       
    // Render stars using Unicode character
    const char* starSymbol = "★";  // Filled star Unicode character
    
    for (int i = 0; i < starCount; ++i) {
        if (!starFont) continue;
        
        int x = startX + (i * spacing);
        
        // Render the star
        SDL_Color goldColor = {255, 215, 0, 255};
        SDL_Surface* surface = TTF_RenderUTF8_Blended(starFont, starSymbol, goldColor);
        
        if (surface) {
            SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
            if (texture) {
                SDL_Rect destRect = {x, startY, surface->w, surface->h};
                SDL_RenderCopy(renderer, texture, nullptr, &destRect);
                SDL_DestroyTexture(texture);
            }
            SDL_FreeSurface(surface);
        }
    }
    
    // Show max prestige text if at 5 stars
    if (starCount >= 5) {
        renderText("MAX PRESTIGE!", startX + 20, startY + 45, {255, 215, 0, 255});
    }
}

void Game::render() {
    // Clear screen with red background (socialist theme)
    SDL_SetRenderDrawColor(renderer, 139, 0, 0, 255);
    SDL_RenderClear(renderer);

    if (!font || !titleFont) return;

    // Title
    renderText("Socialist Tax Collection Bureau", 250, 10, {255, 215, 0, 255});
    
    // Render prestige stars in upper right
    renderStars();
    
    // Show prestige bonus if any
    if (prestigeStars > 0) {
        std::string prestigeText = std::format("Prestige: {} Stars ({}x Bonus)", 
            prestigeStars, static_cast<int>(getPrestigeBonus()));
        renderText(prestigeText, 850, 50, {255, 100, 255, 255});  // Purple/magenta color
    }

    // Main stats
    std::string taxText = std::format("Total Taxes: {} Rubles", formatNumber(totalTaxes));
    std::string tpsText = std::format("Per Second: {}/s", formatNumber(taxesPerSecond));
    std::string cpcText = std::format("Per Click: {}", formatNumber(manualTaxPerClick));
    std::string lifetimeText = std::format("Lifetime: {} Rubles", formatNumber(lifetimeTaxes));
    
    renderText(taxText, 50, 50, {255, 255, 255, 255});
    renderText(tpsText, 50, 75, {200, 200, 200, 255});
    renderText(cpcText, 50, 100, {200, 200, 200, 255});
    renderText(lifetimeText, 50, 125, {180, 180, 180, 255});

    // Save/Load/Reset/Ascend instructions
    if (ascensionConfirmationPending) {
        SDL_Color ascendColor = {255, 100, 255, 255};
        std::string confirmText = std::format("!! PRESS 'A' AGAIN TO ASCEND ({} STARS) !!", playerLevel / 100);
        renderText(confirmText, 350, 770, ascendColor);
    } else if (resetConfirmationPending) {
        SDL_Color warningColor = {255, 50, 50, 255};
        renderText("!! PRESS 'R' AGAIN TO CONFIRM RESET !!", 350, 770, warningColor);
    } else {
        std::string instructions = "S: Save | L: Load | R: Reset";
        if (canAscend()) {
            instructions += " | A: ASCEND (Available!)";
            renderText(instructions, 300, 770, {255, 215, 0, 255});
        } else {
            instructions += " | A: Ascend (Reach Lv100)";
            renderText(instructions, 300, 770, {180, 180, 180, 255});
        }
    }
    
    // Show notification if active
    if (notificationTimer > 0.0) {
        SDL_Color notifColor = {0, 255, 0, 255};
        renderText(notificationMessage, 400, 700, notifColor);
    }

    // Click button
    SDL_Rect clickButton = {50, 180, 200, 200};
    SDL_SetRenderDrawColor(renderer, 200, 0, 0, 255);
    SDL_RenderFillRect(renderer, &clickButton);
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_RenderDrawRect(renderer, &clickButton);
    
    renderText("COLLECT", 100, 260, {255, 215, 0, 255});
    renderText("TAXES!", 110, 290, {255, 215, 0, 255});

    // Passive Income Upgrades (Left Side)
    renderText("=== PASSIVE INCOME ===", 320, 150, {255, 215, 0, 255});
    for (size_t i = 0; i < upgrades.size(); ++i) {
        const auto& upgrade = upgrades[i];
        int y = UPGRADE_Y_START + static_cast<int>(i) * (UPGRADE_HEIGHT + UPGRADE_SPACING);
        uint64_t cost = upgrade.getCurrentCost();
        bool canAfford = totalTaxes >= cost;

        std::string buttonText = std::format("{} - {} ({})",
            upgrade.name, formatNumber(cost), upgrade.owned);

        // Main upgrade button (narrower to fit MAX button)
        renderButton(buttonText, 320, y, 390, UPGRADE_HEIGHT, canAfford);
        
        // MAX button
        int maxPurchasable = calculateMaxPurchasable(
            upgrade.baseCost, upgrade.costMultiplier, upgrade.owned, totalTaxes);
        bool canBuyMax = maxPurchasable > 0;
        renderButton("MAX", 720, y, 50, UPGRADE_HEIGHT, canBuyMax);
        
        // Show effective value with milestone bonus
        double effectiveValue = upgrade.getEffectiveTaxPerSecond();
        int multiplier = static_cast<int>(upgrade.getMilestoneMultiplier());
        
        std::string infoText;
        if (multiplier > 1) {
            infoText = std::format("+{}/s ({}x) | Next: Lv{}", 
                formatNumber(effectiveValue), multiplier, upgrade.getNextMilestoneLevel());
        } else {
            infoText = std::format("+{}/s | Next milestone: Lv10", 
                formatNumber(effectiveValue));
        }
        
        SDL_Color infoColor = (upgrade.owned % 10 == 9) ? 
            SDL_Color{255, 215, 0, 255} : 
            SDL_Color{200, 200, 200, 255};
        
        renderText(infoText, 330, y + 35, infoColor);
    }

    // Click Value Upgrades (Right Side)
    renderText("=== CLICK POWER ===", CLICK_UPGRADE_X_START, 150, {255, 215, 0, 255});
    for (size_t i = 0; i < clickUpgrades.size(); ++i) {
        const auto& upgrade = clickUpgrades[i];
        int y = UPGRADE_Y_START + static_cast<int>(i) * (UPGRADE_HEIGHT + UPGRADE_SPACING);
        uint64_t cost = upgrade.getCurrentCost();
        bool canAfford = totalTaxes >= cost;

        std::string buttonText = std::format("{} - {} ({})",
            upgrade.name, formatNumber(cost), upgrade.owned);

        // Main upgrade button (narrower to fit MAX button)
        renderButton(buttonText, CLICK_UPGRADE_X_START, y, 320, UPGRADE_HEIGHT, canAfford);
        
        // MAX button
        int maxPurchasable = calculateMaxPurchasable(
            upgrade.baseCost, upgrade.costMultiplier, upgrade.owned, totalTaxes);
        bool canBuyMax = maxPurchasable > 0;
        renderButton("MAX", CLICK_UPGRADE_X_START + 330, y, 50, UPGRADE_HEIGHT, canBuyMax);
        
        // Show effective value with milestone bonus
        double effectiveValue = upgrade.getEffectiveClickValue();
        int multiplier = static_cast<int>(upgrade.getMilestoneMultiplier());
        
        std::string infoText;
        if (multiplier > 1) {
            infoText = std::format("+{}/click ({}x) | Next: Lv{}", 
                formatNumber(effectiveValue), multiplier, upgrade.getNextMilestoneLevel());
        } else {
            infoText = std::format("+{}/click | Next: Lv10", 
                formatNumber(effectiveValue));
        }
        
        SDL_Color infoColor = (upgrade.owned % 10 == 9) ? 
            SDL_Color{255, 215, 0, 255} : 
            SDL_Color{200, 200, 200, 255};
        
        renderText(infoText, CLICK_UPGRADE_X_START + 10, y + 35, infoColor);
    }

    // Render level bar at the bottom
    renderLevelBar();

    SDL_RenderPresent(renderer);
}

void Game::renderText(const std::string& text, int x, int y, SDL_Color color) {
    if (!font) return;

    SDL_Surface* surface = TTF_RenderText_Blended(font, text.c_str(), color);
    if (!surface) return;

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) {
        SDL_Rect destRect = {x, y, surface->w, surface->h};
        SDL_RenderCopy(renderer, texture, nullptr, &destRect);
        SDL_DestroyTexture(texture);
    }
    SDL_FreeSurface(surface);
}

void Game::renderButton(const std::string& text, int x, int y, int width, int height, bool canAfford) {
    SDL_Rect buttonRect = {x, y, width, height};
    
    // Button background
    if (canAfford) {
        SDL_SetRenderDrawColor(renderer, 0, 100, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    }
    SDL_RenderFillRect(renderer, &buttonRect);

    // Button border
    SDL_SetRenderDrawColor(renderer, 255, 215, 0, 255);
    SDL_RenderDrawRect(renderer, &buttonRect);

    // Button text
    SDL_Color textColor = canAfford ? SDL_Color{255, 255, 255, 255} : SDL_Color{128, 128, 128, 255};
    renderText(text, x + 10, y + 10, textColor);
}

void Game::cleanup() {
    if (starFont) {
        TTF_CloseFont(starFont);
        starFont = nullptr;
    }
    if (titleFont) {
        TTF_CloseFont(titleFont);
        titleFont = nullptr;
    }
    if (font) {
        TTF_CloseFont(font);
        font = nullptr;
    }
    if (renderer) {
        SDL_DestroyRenderer(renderer);
        renderer = nullptr;
    }
    if (window) {
        SDL_DestroyWindow(window);
        window = nullptr;
    }
    
    TTF_Quit();
    SDL_Quit();
}

std::string Game::formatTime(double seconds) const {
    if (seconds < 0.0) {
        return "Never (no income)";
    }
    
    if (seconds < 60.0) {
        return std::format("{:.0f}s", seconds);
    }
    
    if (seconds < 3600.0) {
        int minutes = static_cast<int>(seconds / 60.0);
        int secs = static_cast<int>(seconds) % 60;
        return std::format("{}m {}s", minutes, secs);
    }
    
    if (seconds < 86400.0) {
        int hours = static_cast<int>(seconds / 3600.0);
        int minutes = static_cast<int>((seconds / 60.0)) % 60;
        return std::format("{}h {}m", hours, minutes);
    }
    
    // Days
    int days = static_cast<int>(seconds / 86400.0);
    int hours = static_cast<int>((seconds / 3600.0)) % 24;
    return std::format("{}d {}h", days, hours);
}

std::string Game::formatNumber(double value) const {
    if (value < 0) return "-" + formatNumber(-value);

    // Under 10,000 - show full number
    if (value < 10000.0) {
        if (value < 10.0) {
            return std::format("{:.2f}", value);
        }
        else if (value < 100.0) {
            return std::format("{:.1f}", value);
        }
        else {
            return std::format("{:.0f}", value);
        }
    }

    // Suffixes for large numbers
    static const std::vector<std::pair<double, std::string>> suffixes = {
        {1e63, "Vigintillion"},
        {1e60, "Novemdecillion"},
        {1e57, "Octodecillion"},
        {1e54, "Septendecillion"},
        {1e51, "Sexdecillion"},
        {1e48, "Quindecillion"},
        {1e45, "Quattuordecillion"},
        {1e42, "Tredecillion"},
        {1e39, "Duodecillion"},
        {1e36, "Undecillion"},
        {1e33, "Decillion"},
        {1e30, "Nonillion"},
        {1e27, "Octillion"},
        {1e24, "Septillion"},
        {1e21, "Sextillion"},
        {1e18, "Quintillion"},
        {1e15, "Quadrillion"},
        {1e12, "Trillion"},
        {1e9,  "Billion"},
        {1e6,  "Million"},
        {1e3,  "Thousand"}
    };

    for (const auto& [threshold, suffix] : suffixes) {
        if (value >= threshold) {
            double scaled = value / threshold;
            if (scaled < 10.0) {
                return std::format("{:.2f} {}", scaled, suffix);
            }
            else if (scaled < 100.0) {
                return std::format("{:.1f} {}", scaled, suffix);
            }
            else {
                return std::format("{:.0f} {}", scaled, suffix);
            }
        }
    }

    return std::format("{:.0f}", value);
}

std::string Game::formatNumber(uint64_t value) const {
    return formatNumber(static_cast<double>(value));
}

int Game::calculateMaxPurchasable(double baseCost, double costMultiplier, int currentOwned, double availableMoney) const {
    if (availableMoney <= 0.0 || baseCost <= 0.0) return 0;
    
    int maxLevels = 0;
    double totalCost = 0.0;
    
    // Calculate how many levels we can afford
    // Cost formula: baseCost * (costMultiplier^owned)
    // Sum of geometric series: baseCost * (1 - r^n) / (1 - r) where r = costMultiplier
    
    // Try up to 1000 levels (reasonable limit)
    for (int i = 0; i < 1000; ++i) {
        double nextCost = baseCost * std::pow(costMultiplier, currentOwned + i);
        if (totalCost + nextCost > availableMoney) {
            break;
        }
        totalCost += nextCost;
        maxLevels++;
    }
    
    return maxLevels;
}

void Game::tryPurchaseUpgradeMax(int upgradeIndex) {
    if (upgradeIndex < 0 || upgradeIndex >= upgrades.size()) return;

    Upgrade& upgrade = upgrades[upgradeIndex];
    
    // Calculate how many we can buy
    int levelsToBuy = calculateMaxPurchasable(
        upgrade.baseCost,
        upgrade.costMultiplier,
        upgrade.owned,
        totalTaxes
    );
    
    if (levelsToBuy == 0) return;
    
    // Calculate total cost
    double totalCost = 0.0;
    for (int i = 0; i < levelsToBuy; ++i) {
        totalCost += upgrade.baseCost * std::pow(upgrade.costMultiplier, upgrade.owned + i);
    }
    
    // Purchase all levels
    int oldLevel = upgrade.owned;
    totalTaxes -= totalCost;
    upgrade.owned += levelsToBuy;
    
    // Recalculate total taxes per second with milestone bonuses
    recalculateTaxesPerSecond();
    
    // Check if milestone reached
    int oldMilestone = oldLevel / 10;
    int newMilestone = upgrade.owned / 10;
    if (newMilestone > oldMilestone) {
        std::string msg = std::format("MAX BUY! {} Lv{} (+{} levels) - {}x Bonus!", 
            upgrade.name, upgrade.owned, levelsToBuy, static_cast<int>(upgrade.getMilestoneMultiplier()));
        showSaveNotification(msg);
    } else {
        std::string msg = std::format("Bought {} x{}", upgrade.name, levelsToBuy);
        showSaveNotification(msg);
    }
}

void Game::tryPurchaseClickUpgradeMax(int upgradeIndex) {
    if (upgradeIndex < 0 || upgradeIndex >= clickUpgrades.size()) return;

    ClickUpgrade& upgrade = clickUpgrades[upgradeIndex];
    
    // Calculate how many we can buy
    int levelsToBuy = calculateMaxPurchasable(
        upgrade.baseCost,
        upgrade.costMultiplier,
        upgrade.owned,
        totalTaxes
    );
    
    if (levelsToBuy == 0) return;
    
    // Calculate total cost
    double totalCost = 0.0;
    for (int i = 0; i < levelsToBuy; ++i) {
        totalCost += upgrade.baseCost * std::pow(upgrade.costMultiplier, upgrade.owned + i);
    }
    
    // Purchase all levels
    int oldLevel = upgrade.owned;
    totalTaxes -= totalCost;
    upgrade.owned += levelsToBuy;
    
    // Recalculate click value with milestone bonuses
    recalculateClickValue();
    
    // Check if milestone reached
    int oldMilestone = oldLevel / 10;
    int newMilestone = upgrade.owned / 10;
    if (newMilestone > oldMilestone) {
        std::string msg = std::format("MAX BUY! {} Lv{} (+{} levels) - {}x Click Power!", 
            upgrade.name, upgrade.owned, levelsToBuy, static_cast<int>(upgrade.getMilestoneMultiplier()));
        showSaveNotification(msg);
    } else {
        std::string msg = std::format("Bought {} x{}", upgrade.name, levelsToBuy);
        showSaveNotification(msg);
    }
}

void Game::resetGame() {
    // Reset all game state to initial values
    totalTaxes = 0.0;
    taxesPerSecond = 0.0;
    lifetimeTaxes = 0.0;
    manualTaxPerClick = 1.0;
    
    // Reset level system
    playerLevel = 0;  // Changed from 1 to 0
    currentXP = 0;
    lastTaxesForXP = 0.0;
    
    // Reset all upgrades
    for (auto& upgrade : upgrades) {
        upgrade.owned = 0;
    }
    
    for (auto& upgrade : clickUpgrades) {
        upgrade.owned = 0;
    }
    
#ifdef _DEBUG
    // Reapply debug boosts
    manualTaxPerClick = DEBUG_MANUAL_TAX_PER_CLICK;
    totalTaxes = 0;
    playerLevel = 0;
    SDL_Log("Game reset with DEBUG MODE values");
#endif
    
    // Recalculate everything
    recalculateTaxesPerSecond();
    recalculateClickValue();
    
    resetConfirmationPending = false;
    showSaveNotification("Game Reset! All progress cleared.");
    
    SDL_Log("Game has been reset to initial state");
}

bool Game::canAscend() const {
    // Can ascend when you have at least 1 star (level 100+)
    return playerLevel >= 100;
}

double Game::getPrestigeBonus() const {
    // Each prestige star gives +50% to all production
    return 1.0 + (prestigeStars * PRESTIGE_BONUS_PER_STAR);
}

void Game::ascendGame() {
    if (!canAscend()) {
        showSaveNotification("You need to reach Level 100 to ascend!");
        return;
    }
    
    // Calculate stars earned from current level
    int starsEarned = playerLevel / 100;
    
    // Add to permanent prestige stars
    prestigeStars += starsEarned;
    totalAscensions++;
    
    // Reset progress (similar to resetGame but keep prestige)
    totalTaxes = 0.0;
    taxesPerSecond = 0.0;
    lifetimeTaxes = 0.0;
    manualTaxPerClick = 1.0;
    
    // Reset level system
    playerLevel = 0;
    currentXP = 0;
    lastTaxesForXP = 0.0;
    
    // Reset all upgrades
    for (auto& upgrade : upgrades) {
        upgrade.owned = 0;
    }
    
    for (auto& upgrade : clickUpgrades) {
        upgrade.owned = 0;
    }
    
#ifdef _DEBUG
    // Reapply debug boosts
    manualTaxPerClick = DEBUG_MANUAL_TAX_PER_CLICK;
    totalTaxes = DEBUG_START_MONEY;
    playerLevel = DEBUG_START_LEVEL;
#endif
    
    // Recalculate with new prestige bonus
    recalculateTaxesPerSecond();
    recalculateClickValue();
    
    ascensionConfirmationPending = false;
    
    std::string msg = std::format("ASCENDED! +{} Stars | Total: {} Stars ({}x Bonus)", 
        starsEarned, prestigeStars, static_cast<int>(getPrestigeBonus()));
    showSaveNotification(msg);
    
    SDL_Log("Ascension #%d complete. Total prestige stars: %d", totalAscensions, prestigeStars);
}