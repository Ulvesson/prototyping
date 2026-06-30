#ifndef SCALE_H_
#define SCALE_H_

#include <cstdint>
#include <cmath>
#include <algorithm>

constexpr uint32_t maxScale = 50000000;
constexpr uint32_t minScale = 1;
constexpr float fractionalLODStep = 0.25f; // Step size for fractional LOD levels

// Clamp to nearest fractional LOD step.
float clampLevelOfDetail(float levelOfDetail) {
    if (levelOfDetail < 0.0f) {
        return 0.0f;
    }

    return std::round(levelOfDetail / fractionalLODStep) * fractionalLODStep;
}

// LOD 0 is max scale, LOD maxLOD is min scale
// For each LOD level, the scale is half of the previous level's scale
// Fractional LOD levels are allowed, and the scale is interpolated 
// between the two nearest integer LOD levels
uint32_t calculateScale(float levelOfDetail) {
    if (levelOfDetail == 0) {
        return maxScale;
    } else {
        // Clamp to nearest fractional LOD step.
        float levelOfDetailClamped = clampLevelOfDetail(levelOfDetail);
        // Calculate the scale for fractional LOD levels
        float lodFloor = std::floor(levelOfDetailClamped);
        float lodCeil = std::ceil(levelOfDetailClamped);
        float scaleFloor = maxScale / std::pow(2, lodFloor);
        float scaleCeil = maxScale / std::pow(2, lodCeil);
        float t = levelOfDetailClamped - lodFloor; // Fractional part
        float interpolatedScale = scaleFloor * (1 - t) + scaleCeil * t;
        return static_cast<uint32_t>(std::max(static_cast<float>(minScale), std::min(static_cast<float>(maxScale), interpolatedScale)));
    }
}

float calculateLevelOfDetail(uint32_t scale) {
    if (scale >= maxScale) {
        return 0.0f;
    } else if (scale <= minScale) {
        return std::log2(static_cast<float>(maxScale) / static_cast<float>(minScale));
    } else {
        float lod = std::log2(static_cast<float>(maxScale) / static_cast<float>(scale));
        return lod;
    }
}

#endif  // SCALE_H_
