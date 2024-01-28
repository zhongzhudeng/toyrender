#include <lightwave.hpp>

namespace lightwave {

class PathTracerIntegrator final : public SamplingIntegrator {
    const cref<Scene> m_scene;
    const int m_depth;

public:
    PathTracerIntegrator(const Properties &properties)
        : SamplingIntegrator(properties),
          m_scene(properties.getChild<Scene>("scene")),
          m_depth(properties.get<int>("depth", 2)) {}

    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its;
        Color bsdf_color;
        Color light_color;
        Color weight = Color::white();
        LightSample ls;
        DirectLightSample dls;
        BsdfEval be;
        BsdfSample bs;

        its = m_scene->intersect(ray, rng);
        if (not its)
            return m_scene->evaluateBackground(ray.direction).value;
        if (its.instance->emission())
            return its.evaluateEmission();

        for (int depth = 1; depth < m_depth; depth++) {
            if (not m_scene->hasLights())
                goto cont;
            ls = m_scene->sampleLight(rng);
            if (ls.light->canBeIntersected())
                goto cont;
            dls = ls.light->sampleDirect(its.position, rng);
            if (dls.isInvalid()) [[unlikely]]
                goto cont;
            if (m_scene->intersect(
                    Ray(its.position, dls.wi), dls.distance, rng))
                goto cont;
            be = its.evaluateBsdf(dls.wi);
            if (be.isInvalid())
                goto cont;
            light_color += weight * be.value * dls.weight / ls.probability;
        cont:
            bs = its.sampleBsdf(rng);
            weight *= bs.weight;
            its = m_scene->intersect(Ray(its.position, bs.wi), rng);
            if (not its) {
                bsdf_color = weight * m_scene->evaluateBackground(bs.wi).value;
                break;
            } else if (its.instance->emission()) {
                auto light = its.instance->light();
                if (not light)
                    bsdf_color = weight * its.evaluateEmission();
                break;
            }
        }
        return bsdf_color + light_color;
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

REGISTER_INTEGRATOR(PathTracerIntegrator, "pathtracer")
