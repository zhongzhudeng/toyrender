#include <lightwave.hpp>

namespace lightwave {

class MISPathTracerIntegrator final : public SamplingIntegrator {
    const cref<Scene> m_scene;
    const int m_depth;
    const bool m_nee;
    const bool m_mis;

    float balanceHeuristic(float f, float g) { return f / (f + g); }
    float powerHeuristic(float f, float g) {
        return sqr(f) / (sqr(f) + sqr(g));
    }

public:
    MISPathTracerIntegrator(const Properties &properties)
        : SamplingIntegrator(properties),
          m_scene(properties.getChild<Scene>("scene")),
          m_depth(properties.get<int>("depth", 2)),
          m_nee(properties.get<bool>("nee", true)),
          m_mis(properties.get<bool>("mis", false)) {}

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
            if (not m_nee || not m_scene->hasLights())
                goto cont;
            ls = m_scene->sampleLight(rng);
            if (not m_mis && ls.light->canBeIntersected()) 
                goto cont;
            dls = ls.light->sampleDirect(its.position, rng);
            if (dls.isInvalid())
                goto cont;
            if (m_scene->intersect(
                    Ray(its.position, dls.wi), dls.distance, rng))
                goto cont;
            be = its.evaluateBsdf(dls.wi);
            if (be.isInvalid())
                goto cont;
            light_color +=
                weight * be.value * dls.weight / ls.probability *
                (m_mis && ls.light->canBeIntersected()
                     ? powerHeuristic(dls.pdf * ls.probability, be.pdf)
                     : 1);
        cont:
            bs = its.sampleBsdf(rng);
            weight *= bs.weight;
            auto r = Ray(its.position, bs.wi);
            its = m_scene->intersect(r, rng);
            if (not its) {
                bsdf_color =
                    weight * m_scene->evaluateBackground(r.direction).value;
                break;
            } else if (its.instance->emission()) {
                auto pdf = 1.f;
                auto light = its.instance->light();
                if (light) {
                    auto p_l = its.pdf *
                               m_scene->lightSelectionProbability(light) *
                               sqr(its.t) / its.frame.normal.dot(its.wo);
                    pdf = powerHeuristic(bs.pdf, p_l);
                }
                bsdf_color = weight * its.evaluateEmission() * pdf;
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

REGISTER_INTEGRATOR(MISPathTracerIntegrator, "mispathtracer")
