#include <lightwave.hpp>

#include "pcg32.h"
#include <functional>

namespace lightwave {

/**
 * @brief Generates random numbers uniformely distributed in [0,1), which are all stochastically independent from another.
 * This is the simplest form of random number generation, and will be sufficient for our introduction to Computer Graphics.
 * If you want to reduce the noise in your renders, a simple way is to implement more sophisticated random numbers (e.g.,
 * jittered sampling or blue noise sampling).
 * @see Internally, this sampler uses the PCG32 library to generate random numbers.
 */
class Halton final : public Sampler {
    uint64_t m_seed;
    pcg32 m_pcg;

    inline float next(uint32_t base, uint32_t a) {
        uint32_t limit = ~0u / base - base;
        float invBase = 1.f / base, invBaseM = 1.f;
        uint32_t reversedDigits = 0;
        while (a && reversedDigits < limit) {
            // Extract least significant digit from _a_ and update _reversedDigits_
            uint32_t next = a / base;
            uint32_t digit = a - next * base;
            reversedDigits = reversedDigits * base + digit;
            invBaseM *= invBase;
            a = next;
        }
        return std::min(reversedDigits * invBaseM, 9.999999e-1f);
    }

public:
    Halton(const Properties &properties) : Sampler(properties) {
        m_seed = properties.get<int>("seed", 1337);
    }

    void seed(int sampleIndex) override { m_pcg.seed(m_seed, sampleIndex); }

    void seed(const Point2i &pixel, int sampleIndex) override {
        const uint64_t a = (uint64_t(pixel.x()) << 32) ^ pixel.y();
        m_pcg.seed(m_seed, a);
        m_pcg.seed(m_pcg.nextUInt(), sampleIndex);
    }

    float next() override { return next(2, m_pcg.nextUInt()); }

    Point2 next2D() override {
        auto a = m_pcg.nextUInt();
        return {next(2, a), next(3, a)};
    }

    ref<Sampler> clone() const override {
        return std::make_shared<Halton>(*this);
    }

    std::string toString() const override {
        return tfm::format(
            "Halton[\n"
            "  count = %d\n"
            "]",
            m_samplesPerPixel);
    }
};

}

REGISTER_SAMPLER(Halton, "halton")
