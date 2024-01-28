#include <lightwave.hpp>

namespace lightwave {

class EnvironmentMap final : public BackgroundLight {
    /// @brief The texture to use as background
    const cref<Texture> m_texture;

    /// @brief An optional transform from local-to-world space
    const cref<Transform> m_transform;

public:
    EnvironmentMap(const Properties &properties)
        : m_texture(properties.getChild<Texture>()),
          m_transform(properties.getOptionalChild<Transform>()) {}

    BackgroundLightEval evaluate(const Vector &direction) const override {
        Vector2 warped = Vector2(0, 0);
        Vector local = direction;
        if (m_transform)
            local = m_transform->inverse(local).normalized();
        return {
            .value = m_texture->evaluate(toUV(local)),
            .pdf = 1, //TODO: better sampling
        };
    }

    //TODO: better sampling
    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        Vector direction = squareToUniformSphere(rng.next2D());
        auto E = evaluate(direction);

        return {
            .wi = direction,
            .weight = E.value / Inv4Pi,
            .distance = Infinity,
            .pdf = 1,
            .cosTheta_o = 1,
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
