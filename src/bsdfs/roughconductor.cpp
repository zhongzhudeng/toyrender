#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughConductor final: public Bsdf {
    ref<Texture> m_reflectance;
    ref<Texture> m_roughness;

public:
    RoughConductor(const Properties &properties) {
        m_reflectance = properties.get<Texture>("reflectance");
        m_roughness   = properties.get<Texture>("roughness");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // Using the squared roughness parameter results in a more gradual
        // transition from specular to rough. For numerical stability, we avoid
        // extremely specular distributions (alpha values below 10^-3)
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));

        auto wm = (wi + wo).normalized();
        auto R = m_reflectance->evaluate(uv);
        auto D = microfacet::evaluateGGX(alpha, wm);
        auto G1_wi = microfacet::smithG1(alpha, wm, wi),
             G1_wo = microfacet::smithG1(alpha, wm, wo);
        auto det_reflection = microfacet::detReflection(wm, wo);
        return {.value = R * D * G1_wi * G1_wo * det_reflection};
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        auto wm = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        auto R = m_reflectance->evaluate(uv);
        auto wi = reflect(wo, wm);
        auto G1_wi = Frame::cosTheta(wo) > 0
                         ? microfacet::smithG1(alpha, wm, wi)
                         : -microfacet::smithG1(alpha, wm, wi);
        return {.wi = wi, .weight = R * G1_wi};
    }

    std::string toString() const override {
        return tfm::format("RoughConductor[\n"
                           "  reflectance = %s,\n"
                           "  roughness = %s\n"
                           "]",
                           indent(m_reflectance), indent(m_roughness));
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
