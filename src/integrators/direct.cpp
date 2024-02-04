#include "lightwave/instance.hpp"
#include "lightwave/integrator.hpp"
#include "lightwave/light.hpp"
#include "lightwave/registry.hpp"

namespace lightwave {

class DirectIntegrator final : public SamplingIntegrator {
    const cref<Scene> m_scene;

public:
    DirectIntegrator(const Properties &properties)
        : SamplingIntegrator(properties),
          m_scene(properties.getChild<Scene>("scene")) {}

    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its = m_scene->intersect(ray, rng);
        if (not its)
            return m_scene->evaluateBackground(ray.direction).value;
        if (its.instance->emission())
            return its.evaluateEmission();

        Color bsdf_color = Color::black(), light_color = Color::black();
        LightSample ls;
        DirectLightSample dls;
        BsdfEval be;
        BsdfSample bs;

        if (not m_scene->hasLights())
            goto cont;
        ls = m_scene->sampleLight(rng);
        if (ls.light->canBeIntersected())
            goto cont;
        dls = ls.light->sampleDirect(its.position, rng);
        if (dls.isInvalid()) [[unlikely]]
            goto cont;
        if (m_scene->intersect(Ray(its.position, dls.wi), dls.distance, rng))
            goto cont;
        be = its.evaluateBsdf(dls.wi);
        if (be.isInvalid()) [[unlikely]]
            goto cont;
        light_color += be.value * dls.weight / ls.probability;
    cont:
        bs = its.sampleBsdf(rng);
        if (bs.isInvalid())
            return light_color;
        its = m_scene->intersect(Ray(its.position, bs.wi), rng);
        if (not its)
            bsdf_color += bs.weight * m_scene->evaluateBackground(bs.wi).value;
        else if (its.instance->emission()) {
            auto light = its.instance->light();
            if (not light)
                bsdf_color += bs.weight * its.evaluateEmission();
        }

        return light_color + bsdf_color;
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging.
    std::string toString() const override {
        return tfm::format(
            "DirectIntegrator[\n"
            "  sampler = %s,\n"
            "  image = %s,\n"
            "]",
            indent(m_sampler),
            indent(m_image));
    }
};

}

REGISTER_INTEGRATOR(DirectIntegrator, "direct")
