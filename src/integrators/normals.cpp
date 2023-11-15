#include "lightwave/color.hpp"
#include "lightwave/core.hpp"
#include "lightwave/math.hpp"
#include <lightwave.hpp>

namespace lightwave {

class NormalsIntegrator : public SamplingIntegrator {
    bool m_remap;
    ref<Scene> m_scene;

public:
    NormalsIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {
        m_remap = properties.get<bool>("remap", true);
        m_scene = properties.getChild<Scene>("scene");
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        auto its = m_scene->intersect(ray, rng);
        Vector d;
        if (its)
            d = its.frame.normal;
        else
            d = Vector(0);
        if (m_remap)
            d = (d + Vector(1)) / 2;
        return Color(d);
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging.
    std::string toString() const override {
        return tfm::format(
            "NormalsIntegrator[\n"
            "  sampler = %s,\n"
            "  image = %s,\n"
            "]",
            indent(m_sampler),
            indent(m_image));
    }
};

}

REGISTER_INTEGRATOR(NormalsIntegrator, "normals")
