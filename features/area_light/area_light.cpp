#include <lightwave.hpp>

namespace lightwave {
class AreaLight final: public Light {
    ref<Instance> m_instance;
    Color m_intensity;

public:
    AreaLight(const Properties &properties) {
        m_instance = properties.getChild<Instance>();
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        AreaSample sample = m_instance->sampleArea(rng);
        Vector w_i = (sample.position - origin).normalized();
        auto cosTheta = std::clamp(-sample.frame.normal.dot(w_i), 0.f, 1.f);
        auto emission = m_instance.get()->emission()->evaluate(sample.uv, w_i).value;
        return {.wi = w_i,
                .weight =
                    emission * cosTheta / (w_i.lengthSquared() * sample.pdf),
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