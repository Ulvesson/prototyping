#include "scale.h"

#include <iostream>

int main() {
    // Test the calculateScale function with various LOD levels
    float lodLevels[] = {0.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f,
        10.0f, 11.0f, 12.0f, 13.0f, 14.0f, 15.0f, 16.0f, 17.0f, 18.0f, 19.0f, 20.0f, 
        21.0f, 22.0f, 23.0f, 24.0f, 25.0f};
        
    for (float lod : lodLevels) {
        uint32_t scale = calculateScale(lod);
        std::cout << "LOD Level: " << lod << ", Scale: " << scale << std::endl;
    }

    // Test the calculateLevelOfDetail function with various scales
    uint32_t scales[] = {50000000, 25000000, 12500000, 6250000, 3125000, 1562500, 781};

    for (uint32_t scale : scales) {
        float lod = calculateLevelOfDetail(scale);
        std::cout << "Scale: " << scale << ", LOD Level: " << lod << std::endl;
    }

    // Test the clampLevelOfDetail function with various LOD levels
    float lodLevelsToClamp[] = {-1.0f, 0.0f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f};

    for (float lod : lodLevelsToClamp) {
        float clampedLOD = clampLevelOfDetail(lod);
        std::cout << "Original LOD: " << lod << ", Clamped LOD: " << clampedLOD << std::endl;
    }

    return 0;
}
