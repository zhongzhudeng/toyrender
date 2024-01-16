#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughDielectric final : public Bsdf {
    const ref<const Texture> m_ior;
    const ref<const Texture> m_reflectance;
    const ref<const Texture> m_transmittance;
    const ref<const Texture> m_roughness;

public:
    RoughDielectric(const Properties &properties)
        : m_ior(properties.get<Texture>("ior")),
          m_reflectance(properties.get<Texture>("reflectance")),
          m_transmittance(properties.get<Texture>("transmittance")),
          m_roughness(properties.get<Texture>("roughness")) {}

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting `wo' is zero, hence we can just ignore
        // that case and always return black
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        const auto wm = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        const float eta = Frame::cosTheta(wo) > 0 ? m_ior->scalar(uv)
                                                  : 1.f / m_ior->scalar(uv);
        const auto f = fresnelDielectric(wo.dot(wm), eta);
        Vector wi;
        Color w;

        if (rng.next() <= f)
            wi = reflect(wo, wm), w = m_reflectance->evaluate(uv);
        else
            wi = refract(wo, wm, eta),
            w = m_transmittance->evaluate(uv) / sqr(eta);

        const auto G1_wi = microfacet::smithG1(alpha, wm, wi);
        return {wi, w * G1_wi * Frame::absCosTheta(wi)};
    }

    std::string toString() const override {
        return tfm::format(
            "RoughDielectric[\n"
            "  ior           = %s,\n"
            "  reflectance   = %s,\n"
            "  transmittance = %s\n"
            "  roughness     = %s\n"
            "]",
            indent(m_ior),
            indent(m_reflectance),
            indent(m_transmittance),
            indent(m_roughness));
    }
};

} // namespace lightwave

REGISTER_BSDF(RoughDielectric, "roughdielectric")
