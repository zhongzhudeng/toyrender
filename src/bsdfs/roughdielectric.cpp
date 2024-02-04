#include "lightwave/bsdf.hpp"
#include "lightwave/properties.hpp"
#include "lightwave/registry.hpp"
#include "lightwave/sampler.hpp"
#include "lightwave/texture.hpp"

#include "fresnel.hpp"
#include "microfacet.hpp"

namespace lightwave {

class RoughDielectric final : public Bsdf {
    const cref<Texture> m_ior;
    const cref<Texture> m_reflectance;
    const cref<Texture> m_transmittance;
    const cref<Texture> m_roughness;

public:
    RoughDielectric(const Properties &properties)
        : m_ior(properties.get<Texture>("ior")),
          m_reflectance(properties.get<Texture>("reflectance")),
          m_transmittance(properties.get<Texture>("transmittance")),
          m_roughness(properties.get<Texture>("roughness")) {}

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        const auto cosTheta_o = Frame::cosTheta(wo),
                   cosTheta_i = Frame::cosTheta(wi);
        if (std::abs(cosTheta_i) < Epsilon || std::abs(cosTheta_o) < Epsilon)
            [[unlikely]]
            return BsdfEval::invalid();

        const auto eta =
            cosTheta_o > 0 ? m_ior->scalar(uv) : 1.f / m_ior->scalar(uv);
        const bool reflect = cosTheta_i * cosTheta_o > 0;
        const auto etap = reflect ? 1 : eta;
        Vector wm = wi * etap + wo;
        wm = std::copysign(1.f, Frame::cosTheta(wm)) * wm.normalized();

        const auto f = fresnelDielectric(wo.dot(wm), eta);
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        const auto G = microfacet::smithG1(alpha, wm, wi) *
                       microfacet::smithG1(alpha, wm, wo);
        auto D = microfacet::evaluateGGX(alpha, wm);
        if (reflect) {
            const auto R = m_reflectance->evaluate(uv);
            return {
                .value = R * f * G * D * microfacet::detReflection(wm, wo),
                .pdf = microfacet::pdfGGXVNDF(alpha, wm, wo) *
                       microfacet::detReflection(wm, wo),
            };
        } else {
            const auto T = m_transmittance->evaluate(uv);
            return {
                .value = T * (1 - f) * G * D *
                         microfacet::detRefraction(wm, wi, wo, eta) /
                         std::abs(cosTheta_o),
                .pdf = microfacet::pdfGGXVNDF(alpha, wm, wo) *
                       microfacet::detRefraction(wm, wi, wo, eta),
            };
        }
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
        float pdf;

        if (rng.next() <= f) {
            wi = reflect(wo, wm);
            w = m_reflectance->evaluate(uv);
            pdf = microfacet::detReflection(wm, wo);
        } else {
            wi = refract(wo, wm, eta);
            w = m_transmittance->evaluate(uv) / sqr(eta);
            pdf = microfacet::detRefraction(wm, wi, wo, eta);
        }

        const auto G1_wi = microfacet::smithG1(alpha, wm, wi);
        return {
            .wi = wi,
            .weight = w * G1_wi * Frame::absCosTheta(wi),
            .pdf = microfacet::pdfGGXVNDF(alpha, wm, wo) * pdf,
        };
    }

    Color albedo(const Point2 &uv, const Vector &wo) const override {
        const float eta = Frame::cosTheta(wo) > 0 ? m_ior->scalar(uv)
                                                  : 1.f / m_ior->scalar(uv);
        const auto f = fresnelDielectric(Frame::absCosTheta(wo), eta);
        return f * m_reflectance->evaluate(uv) +
               (1 - f) * m_transmittance->evaluate(uv) / sqr(eta);
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
