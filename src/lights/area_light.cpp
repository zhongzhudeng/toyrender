#include <lightwave.hpp>

namespace lightwave {
class AreaLight final : public Light {
    const ref<Instance> m_instance;
    const bool m_intersectable;

public:
    AreaLight(const Properties &properties)
        : m_instance(properties.getChild<Instance>()),
          m_intersectable(properties.get<bool>("intersectable", true)) {
        m_instance->setLight(this);
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        const auto sa = m_instance->sampleArea(origin, rng);
        const auto xxp = sa.position - origin;
        const auto wi_l = sa.frame.toLocal(-xxp).normalized();
        const auto emission =
            m_instance.get()->emission()->evaluate(sa.uv, wi_l);
        if (emission.isInvalid())
            return DirectLightSample::invalid();
        const auto cosTheta_o = Frame::cosTheta(wi_l);

        return {
            .wi = xxp.normalized(),
            .weight =
                emission.value * cosTheta_o / (xxp.lengthSquared() * sa.pdf),
            .distance = xxp.length(),
            .pdf = sa.pdf * xxp.lengthSquared() / cosTheta_o,
        };
    }

    bool canBeIntersected() const override { return m_intersectable; }

    std::string toString() const override {
        return tfm::format(
            "AreaLight[\n"
            "]");
    }
};

}

REGISTER_LIGHT(AreaLight, "area")