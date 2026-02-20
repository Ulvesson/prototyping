#include "Game.h"
#include <iostream>
#include <thread>
#include <string>
#include <limits>
#include <sstream>

#ifdef _WIN32
#include <conio.h>

int getKey() {
    return _getch();
}

void restoreTerminal() {}
void setRawMode() {}

#else
#include <termios.h>
#include <unistd.h>

struct termios original_termios;

void restoreTerminal() {
    tcsetattr(STDIN_FILENO, TCSANOW, &original_termios);
}

void setRawMode() {
    tcgetattr(STDIN_FILENO, &original_termios);
    struct termios raw = original_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

int getKey() {
    int ch = getchar();
    return ch;
}

#endif

void displayMenu() {
    std::cout << "\nCommands:\n";
    std::cout << "  [SPACE] Click for resources\n";
    std::cout << "  [U] View/Buy upgrades\n";
    std::cout << "  [S] Show status\n";
    std::cout << "  [Q] Quit game\n";
    std::cout << "\nPress a key: ";
    std::cout.flush();
}

void handleUpgradePurchase(Game& game) {
    // Restore terminal to normal mode for line input
    restoreTerminal();
    
    bool inUpgradeMenu = true;
    
    while (inUpgradeMenu) {
        std::cout << "\n";
        game.displayStatus();
        game.displayUpgrades();
        std::cout << "Enter upgrade number (0 to exit): ";
        std::cout.flush();
        
        std::string line;
        std::getline(std::cin, line);
        
        if (!line.empty()) {
            std::istringstream iss(line);
            int choice;
            if (iss >> choice) {
                if (choice == 0) {
                    std::cout << "Exiting upgrade menu.\n";
                    inUpgradeMenu = false;
                } else if (choice > 0) {
                    game.purchaseUpgrade(choice - 1);
                    // Continue looping so user can buy more
                } else {
                    std::cout << "Invalid choice.\n";
                }
            } else {
                std::cout << "Invalid input.\n";
            }
        }
    }
    
    // Set back to raw mode for key presses
    setRawMode();
}

int main() {
    Game game;
    
    std::cout << "═══════════════════════════════════════════\n";
    std::cout << "  Welcome to IDLE GAME!\n";
    std::cout << "═══════════════════════════════════════════\n";
    std::cout << "Press SPACE to earn resources and buy\n";
    std::cout << "upgrades to generate passive income!\n\n";
    
    game.displayStatus();
    
    // Set terminal to raw mode for immediate key detection
    setRawMode();
    
    while (game.isRunning()) {
        game.update();
        displayMenu();
        
        int input = getKey();
        char ch = static_cast<char>(std::tolower(input));
        
        std::cout << "\n";
        
        switch (ch) {
            case ' ':  // Spacebar
                game.click();
                break;
            case 'u':
                handleUpgradePurchase(game);
                game.displayStatus();
                break;
            case 's':
                game.displayStatus();
                break;
            case 'q':
                std::cout << "Thanks for playing!\n";
                game.quit();
                break;
            default:
                std::cout << "Invalid command!\n";
                break;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Restore terminal before exiting
    restoreTerminal();
    
    return 0;
}