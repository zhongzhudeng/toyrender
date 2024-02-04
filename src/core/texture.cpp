#include "lightwave/texture.hpp"

#include <span>

namespace lightwave {
ScalarImage::ScalarImage() {}
ScalarImage::ScalarImage(std::vector<float> data, Point2i resolution)
    : m_data(data), m_resolution(resolution) {}

Point2i ScalarImage::resolution() const { return m_resolution; }

std::span<float> ScalarImage::data() { return std::span(m_data); }
}