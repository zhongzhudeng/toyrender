#include "fresnel.hpp"
#include "microfacet.hpp"
#include <lightwave.hpp>

namespace lightwave {

class RoughDieletric final : public Bsdf {
    const ref<const Texture> m_ior;
    const ref<const Texture> m_reflectance;
    const ref<const Texture> m_transmittance;
    const ref<const Texture> m_roughness;

public:
    RoughDieletric(const Properties &properties)
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
        auto wm = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        auto h = wm;
        auto eta = m_ior->scalar(uv);
        if (wm.dot(wo) < 0)
            h = -wm, eta = 1.f / eta;
        const auto f = fresnelDielectric(wo.dot(h), eta);
        Vector wi;
        Color w;
        if (rng.next() <= f)
            wi = reflect(wo, h), w = m_reflectance->evaluate(uv);
        else
            wi = refract(wo, h, eta), w = m_transmittance->evaluate(uv);
        w = w * microfacet::smithG1(alpha, wm, wi) *
            microfacet::smithG1(alpha, wm, wo) *
            std::abs(wo.dot(wm) / (wo.z() * wm.z()));
        return {wi, w};
    }

    std::string toString() const override {
        return tfm::format(
            "RoughDieletric[\n"
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

REGISTER_BSDF(RoughDieletric, "roughdieletric")
