#include "lightwave/light.hpp"
#include "lightwave/properties.hpp"
#include "lightwave/registry.hpp"

namespace lightwave {

class PointLight final : public Light {
    Point m_position;
    Color m_intensity;

public:
    PointLight(const Properties &properties) {
        m_position = properties.get<Point>("position");
        m_intensity = properties.get<Color>("power") * Inv4Pi;
    }

    DirectLightSample sampleDirect(const Point &origin,
                                   Sampler &rng) const override {
        Vector ol = m_position - origin;
        return {
            .wi = ol.normalized(),
            .weight = m_intensity / ol.lengthSquared(),
            .distance = ol.length(),
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

REGISTER_LIGHT(PointLight, "point")
