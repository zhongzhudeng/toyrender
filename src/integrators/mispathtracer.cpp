#include "lightwave/instance.hpp"
#include "lightwave/integrator.hpp"
#include "lightwave/light.hpp"
#include "lightwave/registry.hpp"

namespace lightwave {

class MISPathTracerIntegrator final : public SamplingIntegrator {
    const cref<Scene> m_scene;
    const int m_depth;

    float balanceHeuristic(float f, float g) { return f / (f + g); }
    float powerHeuristic(float f, float g) {
        if (std::isfinite(sqr(f))) [[likely]]
            return sqr(f) / (sqr(f) + sqr(g));
        else
            return 1;
    }

public:
    MISPathTracerIntegrator(const Properties &properties)
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
        BackgroundLightEval ble;
        float w;

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
            if (dls.isInvalid())
                goto cont;
            if (m_scene->intersect(
                    Ray(its.position, dls.wi), dls.distance, rng))
                goto cont;
            be = its.evaluateBsdf(dls.wi);
            if (be.isInvalid()) [[unlikely]]
                goto cont;
            w = ls.light->canBeIntersected()
                    ? powerHeuristic(dls.pdf * ls.probability,
                                     be.pdf * dls.cosTheta_o)
                    : 1;
            light_color += w * weight * be.value * dls.weight / ls.probability;
        cont:
            bs = its.sampleBsdf(rng);
            if (bs.isInvalid()) [[unlikely]]
                break;
            weight *= bs.weight;
            auto r = Ray(its.position, bs.wi);
            its = m_scene->intersect(r, rng);
            if (not its) {
                ble = m_scene->evaluateBackground(r.direction);
                w = powerHeuristic(bs.pdf * ble.sinTheta, ble.pdf);
                bsdf_color = w * weight * ble.value;
                break;
            } else if (its.instance->emission()) {
                auto light = its.instance->light();
                if (light) {
                    auto cosTheta_o = its.frame.normal.dot(its.wo);
                    auto pl = m_scene->lightSelectionProbability(light) *
                              its.pdf * sqr(its.t);
                    w = powerHeuristic(bs.pdf * cosTheta_o, pl);
                } else {
                    w = 1;
                }
                bsdf_color = w * weight * its.evaluateEmission();
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
