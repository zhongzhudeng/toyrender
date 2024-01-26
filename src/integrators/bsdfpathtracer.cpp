#include <lightwave.hpp>

namespace lightwave {

class BsdfPathTracerIntegrator final : public SamplingIntegrator {
    const cref<Scene> m_scene;
    const int m_depth;

public:
    BsdfPathTracerIntegrator(const Properties &properties)
        : SamplingIntegrator(properties),
          m_scene(properties.getChild<Scene>("scene")),
          m_depth(properties.get<int>("depth", 2)) {}

    Color Li(const Ray &ray, Sampler &rng) override {
        Intersection its;
        Color weight = Color::white();
        auto r = ray;

        its = m_scene->intersect(r, rng);
        if (not its)
            return m_scene->evaluateBackground(r.direction).value;
        if (its.instance->emission())
            return its.evaluateEmission();

        for (int depth = 1; depth < m_depth; depth++) {
            auto bs = its.sampleBsdf(rng);
            weight *= bs.weight;
            r = Ray(its.position, bs.wi);
            its = m_scene->intersect(r, rng);
            if (not its)
                return weight * m_scene->evaluateBackground(r.direction).value;
            if (its.instance->emission())
                return weight * its.evaluateEmission();
        }
        return Color::black();
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

REGISTER_INTEGRATOR(BsdfPathTracerIntegrator, "bsdfpathtracer")
