#include <lightwave.hpp>

namespace lightwave {

class EnvironmentMap final : public BackgroundLight {
    /// @brief The texture to use as background
    ref<Texture> m_texture;

    /// @brief An optional transform from local-to-world space
    ref<Transform> m_transform;

public:
    EnvironmentMap(const Properties &properties) {
        m_texture = properties.getChild<Texture>();
        m_transform = properties.getOptionalChild<Transform>();
    }

    BackgroundLightEval evaluate(const Vector &direction) const override {
        Vector2 warped = Vector2(0, 0);
        Vector local = direction;
        if (m_transform)
            local = m_transform->inverse(local).normalized();
        float theta = std::acos(local.y());
        float phi = std::acos(local.x() / std::sqrt(1 - local.y() * local.y()));
        phi = local.z() > 0 ? -phi : phi;
        warped.x() = phi * Inv2Pi + 0.5;
        warped.y() = theta * InvPi;
        return {
            .value = m_texture->evaluate(warped),
        };
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        Vector direction = squareToUniformSphere(rng.next2D());
        auto E = evaluate(direction);

        // implement better importance sampling here, if you ever need it
        // (useful for environment maps with bright tiny light sources, like the
        // sun for example)

        return {
            .wi = direction,
            .weight = E.value / Inv4Pi,
            .distance = Infinity,
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
