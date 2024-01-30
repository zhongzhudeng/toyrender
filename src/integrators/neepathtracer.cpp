#include <lightwave.hpp>

namespace lightwave {

class NeePathTracerIntegrator final : public SamplingIntegrator {
    const cref<Scene> m_scene;
    const int m_depth;

public:
    NeePathTracerIntegrator(const Properties &properties)
        : SamplingIntegrator(properties),
          m_scene(properties.getChild<Scene>("scene")),
          m_depth(properties.get<int>("depth", 2)) {}

    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its;
        Color light_color;
        Color weight = Color::white();
        LightSample ls;
        DirectLightSample dls;
        BsdfEval be;

        its = m_scene->intersect(ray, rng);
        if (not its)
            return m_scene->evaluateBackground(ray.direction).value;
        if (its.instance->emission())
            return its.evaluateEmission();

        for (int depth = 1; depth < m_depth; depth++) {
            if (not m_scene->hasLights())
                goto cont;
            ls = m_scene->sampleLight(rng);
            dls = ls.light->sampleDirect(its.position, rng);
            if (dls.isInvalid()) [[unlikely]]
                goto cont;
            if (m_scene->intersect(
                    Ray(its.position, dls.wi), dls.distance, rng))
                goto cont;
            be = its.evaluateBsdf(dls.wi);
            if (be.isInvalid()) [[unlikely]]
                goto cont;
            light_color += weight * be.value * dls.weight / ls.probability;
        cont:
            auto bs = its.sampleBsdf(rng);
            if (bs.isInvalid()) [[unlikely]]
                break;
            weight *= bs.weight;
            its = m_scene->intersect(Ray(its.position, bs.wi), rng);
            if (not its || its.instance->emission())
                break;
        }
        return light_color;
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging.
    std::string toString() const override {
        return tfm::format(
            "MISPathTracerIntegrator[\n"
            "  depth = %d\n"
            "  sampler = %s,\n"
            "  image = %s,\n"
            "]",
            m_depth,
            indent(m_sampler),
            indent(m_image));
    }
};

}

REGISTER_INTEGRATOR(NeePathTracerIntegrator, "neepathtracer")
