#include <lightwave.hpp>

namespace lightwave {

class DirectionalLight final : public Light {
    const Vector m_direction;
    const Color m_intensity;

public:
    DirectionalLight(const Properties &properties)
        : m_direction(properties.get<Vector>("direction").normalized()),
          m_intensity(properties.get<Color>("intensity")) {}

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        return {
            .wi = m_direction,
            .weight = m_intensity,
            .distance = Infinity,
            .pdf = 0,
            .cosTheta_o = 0,
        };
    }

    bool canBeIntersected() const override { return false; }

    std::string toString() const override {
        return tfm::format(
            "PointLight[\n"
            "]");
    }
};

} // namespace lightwave

REGISTER_LIGHT(DirectionalLight, "directional")
