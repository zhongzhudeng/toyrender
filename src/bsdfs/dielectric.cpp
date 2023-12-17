#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric final: public Bsdf {
    const ref<const Texture> m_ior;
    const ref<const Texture> m_reflectance;
    const ref<const Texture> m_transmittance;

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
        Vector n(0, 0, 1);
        float eta = m_ior->scalar(uv);
        if (wo.dot(n) < 0)
            n = -n, eta = 1.f / eta;
        const auto f = fresnelDielectric(wo.dot(n), eta);
        if (rng.next() <= f)
            return {.wi = reflect(wo, n),
                    .weight = m_reflectance->evaluate(uv)};
        else
            return {.wi = refract(wo, n, eta),
                    .weight = m_transmittance->evaluate(uv) / sqr(eta)};
    }

    std::string toString() const override {
        return tfm::format("Dielectric[\n"
                           "  ior           = %s,\n"
                           "  reflectance   = %s,\n"
                           "  transmittance = %s\n"
                           "]",
                           indent(m_ior), indent(m_reflectance),
                           indent(m_transmittance));
    }
};

} // namespace lightwave

REGISTER_BSDF(Dielectric, "dielectric")
