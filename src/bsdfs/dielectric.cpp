#include "fresnel.hpp"
#include <lightwave.hpp>

namespace lightwave {

class Dielectric final: public Bsdf {
    ref<Texture> m_ior;
    ref<Texture> m_reflectance;
    ref<Texture> m_transmittance;

public:
    Dielectric(const Properties &properties) {
        m_ior           = properties.get<Texture>("ior");
        m_reflectance   = properties.get<Texture>("reflectance");
        m_transmittance = properties.get<Texture>("transmittance");
    }

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
        if (Frame::cosTheta(wo) < 0.f)
            n = -n, eta = 1.f / eta;
        auto wi = reflect(wo, n), wr = refract(wo, n, eta);
        if (wr.isZero())
            return {.wi = wi, .weight = m_reflectance->evaluate(uv)};
        float cosThetaI = Frame::cosTheta(wo) < 0.f ? Frame::absCosTheta(wr)
                                                    : Frame::absCosTheta(wo);
        auto f = fresnelDielectric(cosThetaI, m_ior->scalar(uv));
        if (rng.next() <= f)
            return {.wi = wi, .weight = m_reflectance->evaluate(uv)};
        else
            return {.wi = wr,
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
