#include "lightwave/math.hpp"
#include <cmath>
#include <lightwave.hpp>

namespace lightwave {

/**
 * @brief A perspective camera with a given field of view angle and transform.
 * 
 * In local coordinates (before applying m_transform), the camera looks in positive z direction [0,0,1].
 * Pixels on the left side of the image ( @code normalized.x < 0 @endcode ) are directed in negative x
 * direction ( @code ray.direction.x < 0 ), and pixels at the bottom of the image ( @code normalized.y < 0 @endcode )
 * are directed in negative y direction ( @code ray.direction.y < 0 ).
 */
class Perspective : public Camera {
public:
    Perspective(const Properties &properties) : Camera(properties) {
        aspect_ratio_inv = (float)m_resolution.y() / m_resolution.x();
        auto fovAxis = properties.get<std::string>("fovAxis");
        auto fov = properties.get<float>("fov");
        if (fovAxis == "x")
            z = 1.f / std::tan(Deg2Rad * fov / 2);
        else
            z = aspect_ratio_inv / std::tan(Deg2Rad * fov / 2);
    }

    CameraSample sample(const Point2 &normalized, Sampler &rng) const override {
        auto ray_cam =
            Ray(Vector(0.f, 0.f, 0.f),
                Vector(normalized.x(), normalized.y() * aspect_ratio_inv, z));
        auto ray_world = m_transform->apply(ray_cam).normalized();
        return CameraSample{.ray = ray_world, .weight = Color(1.0f)};
    }

    std::string toString() const override {
        return tfm::format(
            "Perspective[\n"
            "  width = %d,\n"
            "  height = %d,\n"
            "  transform = %s,\n"
            "]",
            m_resolution.x(),
            m_resolution.y(),
            indent(m_transform));
    }

private:
    float aspect_ratio_inv, z;
};

}

REGISTER_CAMERA(Perspective, "perspective")
