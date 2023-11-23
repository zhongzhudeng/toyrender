#include <lightwave.hpp>

namespace lightwave {
class AreaLight : public Light {
    ref<Instance> m_instance;

public:
    AreaLight(const Properties &properties) {
        m_instance = properties.getChild<Instance>();
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        AreaSample sample = m_instance->sampleArea(rng);
        Vector w_i = sample.position - origin;
        return {.wi = w_i.normalized(),
                .weight = m_instance->emission()
                              ->evaluate(sample.uv, -w_i.normalized())
                              .value /
                          sample.pdf,
                .distance = w_i.length()};
    }

    bool canBeIntersected() const override { return true; }

    std::string toString() const override {
        return tfm::format(
            "AreaLight[\n"
            "]");
    }
};

}

REGISTER_LIGHT(AreaLight, "light")