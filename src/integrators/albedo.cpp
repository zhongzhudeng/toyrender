#include "lightwave/color.hpp"
#include "lightwave/core.hpp"
#include "lightwave/math.hpp"
#include <lightwave.hpp>

namespace lightwave {

class AlbedoIntegrator final : public SamplingIntegrator {
    ref<Scene> m_scene;

public:
    AlbedoIntegrator(const Properties &properties)
        : SamplingIntegrator(properties) {
        m_scene = properties.getChild<Scene>("scene");
    }

    Color Li(const Ray &ray, Sampler &rng) override {
        auto its = m_scene->intersect(ray, rng);
        if (not its)
            return m_scene->evaluateBackground(ray.direction).value;
        if (its.instance->emission())
            return its.evaluateEmission();
        return its.evaluateAlbedo();
    }

    /// @brief An optional textual representation of this class, which can be useful for debugging.
    std::string toString() const override {
        return tfm::format(
            "AlbedoIntegrator[\n"
            "  sampler = %s,\n"
            "  image = %s,\n"
            "]",
            indent(m_sampler),
            indent(m_image));
    }
};

}

REGISTER_INTEGRATOR(AlbedoIntegrator, "albedo")
