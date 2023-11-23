#include <lightwave.hpp>

namespace lightwave {

class AreaLightDirectIntegrator : public SamplingIntegrator {
    ref<Scene> m_scene;

public:
    AreaLightDirectIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {
        m_scene = properties.getChild<Scene>("scene");
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its = m_scene->intersect(ray, rng);
        if (not its)
            return m_scene->evaluateBackground(ray.direction).value;

        if (its.instance->emission())
            return its.evaluateEmission();

        Color value;

        BsdfSample bsdf = its.sampleBsdf(rng);
        Ray secondary_ray(its.position, bsdf.wi);
        Intersection secondary_its = m_scene->intersect(secondary_ray, rng);

        if (not secondary_its)
            value += bsdf.weight *
                     m_scene->evaluateBackground(secondary_ray.direction).value;
        else if (secondary_its.instance->emission())
            value += bsdf.weight * secondary_its.evaluateEmission();

        LightSample light = m_scene->sampleLight(rng);
        auto sample = light.light->sampleDirect(its.position, rng);

        // auto f_r = its.instance->evaluate(-ray.direction, its.uv, sample.wi);
        // value += f_r*sample.weight* its.frame.normal.dot(sample.wi)*sample.frame.normal.dot(-sample.wi) /(sample.distance *sample.distance);
        // its.instance->bsdf();
        // value += light.


        return Color::black();
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

REGISTER_INTEGRATOR(AreaLightDirectIntegrator, "arealightdirect")
