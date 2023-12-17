#include <lightwave.hpp>

#include "fresnel.hpp"
#include "microfacet.hpp"

namespace lightwave {

struct DiffuseLobe {
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        return {.value = Frame::cosTheta(wi) * color * InvPi};
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        auto wi = squareToCosineHemisphere(rng.next2D());
        return {.wi = wi, .weight = color};
    }
};

struct MetallicLobe {
    float alpha;
    Color color;

    BsdfEval evaluate(const Vector &wo, const Vector &wi) const {
        auto wm = (wi + wo).normalized();
        auto D = microfacet::evaluateGGX(alpha, wm);
        auto G1_wi = microfacet::smithG1(alpha, wm, wi),
             G1_wo = microfacet::smithG1(alpha, wm, wo);
        return {.value = color * D * G1_wi * G1_wo *
                         std::copysign(1.f, Frame::cosTheta(wi)) /
                         (4 * Frame::absCosTheta(wo))};
    }

    BsdfSample sample(const Vector &wo, Sampler &rng) const {
        auto wm = microfacet::sampleGGXVNDF(alpha, wo, rng.next2D());
        auto wi = reflect(wo, wm);
        auto G1_wi = microfacet::smithG1(alpha, wm, wi);
        return {.wi = wi,
                .weight =
                    color * G1_wi * std::copysign(1.f, Frame::cosTheta(wi))};
    }
};

class Principled final: public Bsdf {
    ref<Texture> m_baseColor;
    ref<Texture> m_roughness;
    ref<Texture> m_metallic;
    ref<Texture> m_specular;

    struct Combination {
        float diffuseSelectionProb;
        DiffuseLobe diffuse;
        MetallicLobe metallic;
    };

    Combination combine(const Point2 &uv, const Vector &wo) const {
        const auto baseColor = m_baseColor->evaluate(uv);
        const auto alpha = std::max(float(1e-3), sqr(m_roughness->scalar(uv)));
        const auto specular = m_specular->scalar(uv);
        const auto metallic = m_metallic->scalar(uv);
        const auto F =
            specular * schlick((1 - metallic) * 0.08f, Frame::cosTheta(wo));

        const DiffuseLobe diffuseLobe = {
            .color = (1 - F) * (1 - metallic) * baseColor,
        };
        const MetallicLobe metallicLobe = {
            .alpha = alpha,
            .color = F * Color(1) + (1 - F) * metallic * baseColor,
        };

        const auto diffuseAlbedo = diffuseLobe.color.mean();
        const auto totalAlbedo =
            diffuseLobe.color.mean() + metallicLobe.color.mean();
        return {
            .diffuseSelectionProb =
                totalAlbedo > 0 ? diffuseAlbedo / totalAlbedo : 1.0f,
            .diffuse  = diffuseLobe,
            .metallic = metallicLobe,
        };
    }

public:
    Principled(const Properties &properties) {
        m_baseColor = properties.get<Texture>("baseColor");
        m_roughness = properties.get<Texture>("roughness");
        m_metallic  = properties.get<Texture>("metallic");
        m_specular  = properties.get<Texture>("specular");
    }

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        const auto combination = combine(uv, wo);
        return {.value = combination.diffuse.evaluate(wo, wi).value +
                         combination.metallic.evaluate(wo, wi).value};
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const auto combination = combine(uv, wo);
        BsdfSample bsdf;
        if (rng.next() < combination.diffuseSelectionProb ) {
            bsdf = combination.diffuse.sample(wo, rng);
            bsdf.weight /= combination.diffuseSelectionProb;
        } else {
            bsdf = combination.metallic.sample(wo, rng);
            bsdf.weight /= (1 - combination.diffuseSelectionProb);
        }

        return bsdf;
    }

    std::string toString() const override {
        return tfm::format("Principled[\n"
                           "  baseColor = %s,\n"
                           "  roughness = %s,\n"
                           "  metallic  = %s,\n"
                           "  specular  = %s,\n"
                           "]",
                           indent(m_baseColor), indent(m_roughness),
                           indent(m_metallic), indent(m_specular));
    }
};

} // namespace lightwave

REGISTER_BSDF(Principled, "principled")
