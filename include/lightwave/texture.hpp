/**
 * @file texture.hpp
 * @brief Contains the Texture interface, which models spatially varying properties of materials.
 */

#pragma once

#include "lightwave/core.hpp"
#include "lightwave/math.hpp"

#include <span>

namespace lightwave {

class ScalarImage {
    std::vector<float> m_data;
    Point2i m_resolution;

public:
    ScalarImage();
    ScalarImage(std::vector<float> data, Point2i resolution);
    Point2i resolution() const;
    std::span<float> data();
};

/// @brief Models spatially varying material properties (e.g., images or procedural noise).
class Texture : public Object {
public:
    /**
     * @brief Returns the color at a given texture coordinate.
     * For most applications, the input point will lie in the unit square [0,1)^2, but points outside this
     * domain are also allowed.
     */
    virtual Color evaluate(const Point2 &uv) const = 0;
    /**
     * @brief Returns a scalar value at a given texture coordinate.
     * For most applications, the input point will lie in the unit square [0,1)^2, but points outside this
     * domain are also allowed.
     */
    virtual float scalar(const Point2 &uv) const = 0;

    virtual ScalarImage scalar() const = 0;
};

}
