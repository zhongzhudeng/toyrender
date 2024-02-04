#include "lightwave/distribution.hpp"
#include "lightwave/light.hpp"
#include "lightwave/properties.hpp"
#include "lightwave/registry.hpp"
#include "lightwave/sampler.hpp"
#include "lightwave/texture.hpp"
#include "lightwave/transform.hpp"

namespace lightwave {

inline Vector sphericalDirection(float sinTheta, float cosTheta, float phi) {
    return Vector(std::clamp(sinTheta, -1.f, 1.f) * std::cos(phi),
                  std::clamp(cosTheta, -1.f, 1.f),
                  std::clamp(sinTheta, -1.f, 1.f) * std::sin(phi));
}

inline float sphericalTheta(const Vector &w) { return safe_acos(w.y()); }

inline float sphericalPhi(const Vector &w) { return std::atan2(w.z(), w.x()); }

inline Point2 to_uv(const Vector &w) {
    float u = 0.5 - sphericalPhi(w) * Inv2Pi;
    float v = sphericalTheta(w) * InvPi;
    return Point2(u, v);
}

inline Vector to_cartisian(Point2 &uv) {
    float theta = uv.y() * Pi;
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);
    float phi = (0.5 - uv.x()) * 2 * Pi;
    return sphericalDirection(sinTheta, cosTheta, phi);
}

class EnvironmentMap final : public BackgroundLight {
    /// @brief The texture to use as background
    const cref<Texture> m_texture;

    /// @brief An optional transform from local-to-world space
    const cref<Transform> m_transform;

    ScalarImage m_scalar;
    Distribution2D m_distr;

public:
    EnvironmentMap(const Properties &properties)
        : m_texture(properties.getChild<Texture>()),
          m_transform(properties.getOptionalChild<Transform>()) {
        m_scalar = m_texture->scalar();
        m_distr = Distribution2D(m_scalar);
    }

    BackgroundLightEval evaluate(const Vector &direction) const override {
        Vector local = direction;
        if (m_transform)
            local = m_transform->inverse(local).normalized();
        Point2 uv = to_uv(local);
        auto e = m_texture->evaluate(uv);
        uv.y() = 1 - uv.y();
        auto pdf = m_distr.pdf(uv);
        float theta = uv.y() * Pi;
        float sinTheta = std::sin(theta);
        if (pdf < 1e-6)
            e = Color::black();

        return {
            .value = e,
            .pdf = pdf / (2 * Pi * Pi),
            .sinTheta = sinTheta,
        };
    }
    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        auto s = m_distr.sampleContinous(rng);
        auto uv = s.uv;
        float theta = uv.y() * Pi;
        float sinTheta = std::sin(theta);
        uv.y() = 1 - uv.y();
        auto wi = to_cartisian(uv);
        if (m_transform)
            wi = m_transform->apply(wi);
        auto e = m_texture->evaluate(uv);
        auto w = e * 2 * Pi * Pi * sinTheta / s.pdf;
        if (s.pdf < 1e-6)
            w = Color::black();

        return {
            .wi = wi,
            .weight = w,
            .distance = Infinity,
            .pdf = s.pdf / (2 * Pi * Pi),
            .cosTheta_o = sinTheta,
        };
    }

    std::string toString() const override {
        return tfm::format(
            "EnvironmentMap[\n"
            "  texture = %s,\n"
            "  transform = %s\n"
            "]",
            indent(m_texture),
            indent(m_transform));
    }
};

} // namespace lightwave

REGISTER_LIGHT(EnvironmentMap, "envmap")
