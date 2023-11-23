#include <lightwave.hpp>

namespace lightwave {

class DirectIntegrator : public SamplingIntegrator {
    ref<Scene> m_scene;

public:
    DirectIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {
        m_scene = properties.getChild<Scene>("scene");
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its = m_scene->intersect(ray, rng);
        if (not its)
            return m_scene->evaluateBackground(ray.direction).value;

        if (its.instance->emission())
            return its.evaluateEmission();

        BsdfSample bsdf = its.sampleBsdf(rng);
        Ray secondary_ray(its.position, bsdf.wi);
        Intersection secondary_its = m_scene->intersect(secondary_ray, rng);

        if (not secondary_its)
            return bsdf.weight *
                   m_scene->evaluateBackground(secondary_ray.direction).value;

        if (secondary_its.instance->emission())
            return bsdf.weight * secondary_its.evaluateEmission();

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

REGISTER_INTEGRATOR(DirectIntegrator, "direct")
