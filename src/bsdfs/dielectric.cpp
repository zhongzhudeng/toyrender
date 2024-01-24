#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric final : public Bsdf {
    const cref<Texture> m_ior;
    const cref<Texture> m_reflectance;
    const cref<Texture> m_transmittance;

public:
    Dielectric(const Properties &properties)
        : m_ior(properties.get<Texture>("ior")),
          m_reflectance(properties.get<Texture>("reflectance")),
          m_transmittance(properties.get<Texture>("transmittance")) {}

    BsdfEval evaluate(const Point2 &uv, const Vector &wo,
                      const Vector &wi) const override {
        // the probability of a light sample picking exactly the direction `wi'
        // that results from reflecting or refracting `wo' is zero, hence we can
        // just ignore that case and always return black
        return BsdfEval::invalid();
    }

    BsdfSample sample(const Point2 &uv, const Vector &wo,
                      Sampler &rng) const override {
        const Vector n(0, 0, 1);
        const float eta = Frame::cosTheta(wo) > 0 ? m_ior->scalar(uv)
                                                  : 1.f / m_ior->scalar(uv);
        const float sgn = copysign(1, Frame::cosTheta(wo));

        const auto f = fresnelDielectric(Frame::absCosTheta(wo), eta);
        if (rng.next() <= f)
            return {.wi = reflect(wo, sgn * n),
                    .weight = m_reflectance->evaluate(uv),
                    .pdf = 0};
        else
            return {.wi = refract(wo, sgn * n, eta),
                    .weight = m_transmittance->evaluate(uv) / sqr(eta),
                    .pdf = 0};
    }

    std::string toString() const override {
        return tfm::format(
            "Dielectric[\n"
            "  ior           = %s,\n"
            "  reflectance   = %s,\n"
            "  transmittance = %s\n"
            "]",
            indent(m_ior),
            indent(m_reflectance),
            // const auto f = fresnelDielectric(Frame::absCosTheta(wo), eta);
            indent(m_transmittance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Dielectric, "dielectric")
