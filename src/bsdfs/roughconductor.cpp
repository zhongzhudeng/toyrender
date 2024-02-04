#include "lightwave/bsdf.hpp"
#include "lightwave/properties.hpp"
#include "lightwave/registry.hpp"
#include "lightwave/sampler.hpp"
#include "lightwave/texture.hpp"

#include "microfacet.hpp"

namespace lightwave {

class RoughConductor final : public Bsdf {
    const cref<Texture> m_reflectance;
    const cref<Texture> m_roughness;

public:
    RoughConductor(const Properties &properties)
        : m_reflectance(properties.get<Texture>("reflectance")),
          m_roughness(properties.get<Texture>("roughness")) {}

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        if (Frame::cosTheta(wi) <= 0 || Frame::cosTheta(wo) <= 0) [[unlikely]]
            return BsdfEval::invalid();

        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        auto wm = (wi + wo).normalized();
        auto R = m_reflectance->evaluate(uv);
        auto D = microfacet::evaluateGGX(alpha, wm);
        auto G1_wi = microfacet::smithG1(alpha, wm, wi),
             G1_wo = microfacet::smithG1(alpha, wm, wo);
        return {
            .value = R * D * G1_wi * G1_wo / (4 * Frame::cosTheta(wo)),
            .pdf = microfacet::pdfGGXVNDF(alpha, wm, wo) *
                   microfacet::detReflection(wm, wo),
        };
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        if (Frame::cosTheta(wo) <= 0) [[unlikely]]
            return BsdfSample::invalid();

        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        auto wm = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        auto R = m_reflectance->evaluate(uv);
        auto wi = reflect(wo, wm);
        auto G1_wi = microfacet::smithG1(alpha, wm, wi);
        return {
            .wi = wi,
            .weight = R * G1_wi,
            .pdf = microfacet::pdfGGXVNDF(alpha, wm, wo) *
                   microfacet::detReflection(wm, wo),
        };
    }

    Color albedo(const Point2 &uv, const Vector &wo) const override {
        return m_reflectance->evaluate(uv);
    }

    std::string toString() const override {
        return tfm::format(
            "RoughConductor[\n"
            "  reflectance = %s,\n"
            "  roughness = %s\n"
            "]",
            indent(m_reflectance),
            indent(m_roughness));
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughConductor, "roughconductor")
