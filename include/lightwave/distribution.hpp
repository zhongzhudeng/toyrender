#pragma once

#include "lightwave/sampler.hpp"
#include "lightwave/texture.hpp"

#include <span>
#include <vector>

namespace lightwave {

struct Eval1D {
    size_t index;
    float pdf;
};

struct Sample1D {
    size_t index;
    float u;
    float pdf;
};

struct Sample2D {
    Point2 uv;
    float pdf;
};

class Distribution1D {
    std::span<float> m_data;
    std::vector<float> m_cdf;
    size_t m_size;
    float m_sum;

public:
    Distribution1D();
    Distribution1D(std::span<float> data);
    Sample1D sampleContinous(Sampler &rng) const;
    float sum() const;

    std::span<float> data() const;
};

class Distribution2D {
    std::vector<Distribution1D> m_conditional;
    Distribution1D m_marginal;
    std::vector<float> m_marginal_data;

    Point2i m_resolution;

public:
    Distribution2D();
    Distribution2D(ScalarImage scalar);

    float pdf(Point2 uv) const;
    Sample2D sampleContinous(Sampler &rng) const;
};

}