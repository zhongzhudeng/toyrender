#include <lightwave.hpp>

namespace lightwave {
class AreaLight final: public Light {
    ref<Instance> m_instance;
    Color m_intensity;

public:
    AreaLight(const Properties &properties) {
        m_instance = properties.getChild<Instance>();
        m_instance->setLight(this);
        m_intensity = properties.get<Color>("power");
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        AreaSample sample = m_instance->sampleArea(rng);
        Vector w_i = sample.position - origin;
        return {.wi = w_i.normalized(),
                .weight = m_intensity / (sample.pdf * w_i.lengthSquared()),
                .distance = w_i.length()};
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format(
            "AreaLight[\n"
            "]");
    }
};

}

REGISTER_LIGHT(AreaLight, "area")