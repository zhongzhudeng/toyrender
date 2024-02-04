#include "lightwave/emission.hpp"
#include "lightwave/properties.hpp"
#include "lightwave/registry.hpp"
#include "lightwave/texture.hpp"

namespace lightwave {

class Lambertian final : public Emission {
    const cref<Texture> m_emission;

public:
    Lambertian(const Properties &properties)
        : m_emission(properties.get<Texture>("emission")) {}

    EmissionEval evaluate(const Point2 &uv, const Vector &wo) const override {
        if (Frame::cosTheta(wo) <= 0)
            return EmissionEval::invalid();
        return EmissionEval{.value = m_emission->evaluate(uv)};
    }

    std::string toString() const override {
        return tfm::format(
            "Lambertian[\n"
            "  emission = %s\n"
            "]",
            indent(m_emission));
    }
};

} // namespace lightwave

REGISTER_EMISSION(Lambertian, "lambertian")
