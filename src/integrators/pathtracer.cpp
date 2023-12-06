#include <lightwave.hpp>

namespace lightwave {

class PathTracerIntegrator final : public SamplingIntegrator {
    const ref<const Scene> m_scene;
    const int m_depth;

public:
    PathTracerIntegrator(const Properties &properties)
        : SamplingIntegrator(properties),
          m_scene(properties.getChild<Scene>("scene")),
          m_depth(properties.get<int>("depth", 2)) {}

    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its, test_its = m_scene->intersect(ray, rng);
        if (not test_its)
            return m_scene->evaluateBackground(ray.direction).value;
        if (test_its.instance->emission())
            return test_its.frame.normal.dot(-ray.direction) > 0
                       ? test_its.evaluateEmission()
                       : Color::black();

        Color bsdf_color = Color::black(), light_color = Color::black(),
              weight = Color::white();
        LightSample ls;
        DirectLightSample dls;

        for (int depth = 1; depth < m_depth; depth++) {
            its = test_its;
            if (not m_scene->hasLights())
                goto cont;
            ls = m_scene->sampleLight(rng);
            if (ls.light->canBeIntersected()) [[unlikely]]
                goto cont;
            dls = ls.light->sampleDirect(its.position, rng);
            if (its.frame.normal.dot(dls.wi) <= 0)
                goto cont;
            if (m_scene->intersect(
                    Ray(its.position, dls.wi), dls.distance, rng))
                goto cont;
            light_color += weight * its.evaluateBsdf(dls.wi).value *
                           dls.weight / ls.probability;
        cont:
            auto bsdf = its.sampleBsdf(rng);
            weight *= bsdf.weight;
            auto r = Ray(its.position, bsdf.wi);
            test_its = m_scene->intersect(r, rng);
            if (not test_its) {
                bsdf_color =
                    weight * m_scene->evaluateBackground(r.direction).value;
                break;
            } else if (test_its.instance->emission()) {
                bsdf_color = test_its.frame.normal.dot(-r.direction) > 0
                                 ? weight * test_its.evaluateEmission()
                                 : Color::black();
                break;
            }
        }
        return bsdf_color + light_color;
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging.
    std::string toString() const override {
        return tfm::format(
            "PathTracerIntegrator[\n"
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
