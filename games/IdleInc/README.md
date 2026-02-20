# Incremental Idle Game

A simple console-based incremental idle game written in C++20.

## Features

- **Click Mechanics**: Click to earn resources manually
- **Passive Income**: Purchase upgrades that generate resources automatically
- **Progressive Upgrades**: 5 different upgrade types with scaling costs
- **Real-time Updates**: Resources accumulate in real-time

## Building

````````
mkdir build && cd build
cmake .. -G Ninja
cmake --build .
./IdleGame

````````

## How to Play

1. Run the executable: `./IdleGame` (or `IdleGame.exe` on Windows)
2. Press **C** to click and earn resources
3. Press **U** to view and purchase upgrades
4. Press **S** to view your current status
5. Press **Q** to quit

## Upgrades

- **Auto-Clicker**: Entry-level passive income
- **Mining Rig**: Moderate passive income
- **Factory**: High passive income
- **Corporation**: Very high passive income
- **Quantum Generator**: Extreme passive income

Each upgrade's cost increases exponentially with each purchase!

## Extending the Game

This is a foundation you can build upon:
- Add save/load functionality
- Implement prestige/reset mechanics
- Add achievements
- Create a GUI with a graphics library
- Add more upgrade tiers and special abilities
