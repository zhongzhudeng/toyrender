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
        const auto sa = m_instance->sampleArea(origin, rng);
        if (sa.isInvalid())
            return DirectLightSample::invalid();

        auto xxp = Vector(sa.position - origin);
        auto wi = xxp.normalized();
        auto wo = sa.frame.toLocal(-wi);
        auto cosTheta_o = Frame::cosTheta(wo);
        if (cosTheta_o <= 0) [[unlikely]]
            return DirectLightSample::invalid();

        auto len2 = xxp.lengthSquared();
        auto len = xxp.length();

        const auto emission = m_instance.get()->emission()->evaluate(sa.uv, wo);
        if (emission.isInvalid())
            return DirectLightSample::invalid();

        return {
            .wi = wi,
            .weight = emission.value / (sa.pdf * len2),
            .distance = len,
            .pdf = sa.pdf * len2,
            .cosTheta_o = cosTheta_o,
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