#include <lightwave.hpp>

namespace lightwave {

class Diffuse final: public Bsdf {
    const ref<const Texture> m_albedo;

public:
    Diffuse(const Properties &properties)
        : m_albedo(properties.get<Texture>("albedo")) {}

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        return {.value = Frame::cosTheta(wi) * m_albedo->evaluate(uv)};
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        auto wi = squareToCosineHemisphere(rng.next2D());
        return BsdfSample{.wi = wi, .weight = m_albedo->evaluate(uv)};
    }

    std::string toString() const override {
        return tfm::format(
            "Diffuse[\n"
            "  albedo = %s\n"
            "]",
            indent(m_albedo));
    }
};

} // namespace lightwave

REGISTER_BSDF(Diffuse, "diffuse")
