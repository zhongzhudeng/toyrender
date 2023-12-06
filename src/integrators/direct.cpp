#include <lightwave.hpp>

namespace lightwave {

class DirectIntegrator final : public SamplingIntegrator {
    const ref<const Scene> m_scene;

public:
    DirectIntegrator(const Properties &properties)
        : SamplingIntegrator(properties),
          m_scene(properties.getChild<Scene>("scene")) {}

    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its, test_its = m_scene->intersect(ray, rng);
        if (not test_its)
            return m_scene->evaluateBackground(ray.direction).value;
        if (test_its.instance->emission())
            return test_its.frame.normal.dot(-ray.direction) > 0
                       ? test_its.evaluateEmission()
                       : Color::black();

        Color bsdf_color, light_color;
        LightSample ls;
        DirectLightSample dls;
        its = test_its;
        if (not m_scene->hasLights())
            goto cont;
        ls = m_scene->sampleLight(rng);
        if (ls.light->canBeIntersected())
            goto cont;
        dls = ls.light->sampleDirect(its.position, rng);
        if (its.frame.normal.dot(dls.wi) <= 0)
            goto cont;
        if (m_scene->intersect(Ray(its.position, dls.wi), dls.distance, rng))
            goto cont;
        light_color =
            its.evaluateBsdf(dls.wi).value * dls.weight / ls.probability;

    cont:
        auto bsdf = its.sampleBsdf(rng);
        auto r = Ray(its.position, bsdf.wi);
        test_its = m_scene->intersect(r, rng);
        if (not test_its)
            bsdf_color =
                bsdf.weight * m_scene->evaluateBackground(r.direction).value;
        else if (test_its.instance->emission())
            bsdf_color = test_its.frame.normal.dot(-r.direction) > 0
                             ? bsdf.weight * test_its.evaluateEmission()
                             : Color::black();
        return bsdf_color + light_color;
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
