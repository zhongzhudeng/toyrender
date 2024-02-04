#include "lightwave/instance.hpp"
#include "lightwave/light.hpp"
#include "lightwave/properties.hpp"
#include "lightwave/registry.hpp"

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
        auto sa = m_instance->sampleArea(origin, rng);
        if (sa.isInvalid()) [[unlikely]]
            return DirectLightSample::invalid();

        auto xxp = Vector(sa.position - origin);
        auto wi = xxp.normalized();
        auto wo = sa.frame.toLocal(-wi);
        auto cosTheta_o = Frame::cosTheta(wo);
        if (cosTheta_o <= 0) [[unlikely]]
            return DirectLightSample::invalid();

        auto len2 = xxp.lengthSquared();
        auto len = xxp.length();

        auto emission = m_instance.get()->emission()->evaluate(sa.uv, wo);
        if (emission.isInvalid()) [[unlikely]]
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