#include <lightwave.hpp>

namespace lightwave {
class AreaLight final : public Light {
    const ref<Instance> m_instance;

public:
    AreaLight(const Properties &properties)
        : m_instance(properties.getChild<Instance>()) {
        m_instance->setLight(this);
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        const auto sa = m_instance->sampleArea(rng);
        const auto xxp = sa.position - origin;
        const auto wi = xxp.normalized();
        const auto wi_l = sa.frame.toLocal(-xxp).normalized();
        const auto emission =
            m_instance.get()->emission()->evaluate(sa.uv, wi_l);
        if (emission.isInvalid())
            return DirectLightSample::invalid();
        const auto cosTheta_o = Frame::absCosTheta(wi_l);

        return {
            .wi = wi,
            .weight =
                emission.value * cosTheta_o / (xxp.lengthSquared() * sa.pdf),
            .distance = xxp.length(),
            .pdf = sa.pdf * xxp.lengthSquared() / cosTheta_o,
        };
    }

    bool canBeIntersected() const override { return true; }

    std::string toString() const override {
        return tfm::format(
            "AreaLight[\n"
            "]");
    }
};

}

REGISTER_LIGHT(AreaLight, "area")