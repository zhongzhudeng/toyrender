#include "lightwave/math.hpp"
#include <cmath>
#include <lightwave.hpp>

namespace lightwave {

class ThinLens : public Camera {
public:
    ThinLens(const Properties &properties) : Camera(properties) {
        aspect_ratio_inv = (float)m_resolution.y() / m_resolution.x();
        auto fovAxis = properties.get<std::string>("fovAxis");
        auto fov = properties.get<float>("fov");
        if (fovAxis == "x")
            z = 1.f / std::tan(Deg2Rad * fov / 2);
        else
            z = aspect_ratio_inv / std::tan(Deg2Rad * fov / 2);
        lens_radius = properties.get<float>("lensRadius", 1);
        focal_distance = properties.get<float>("focalDistance", 1);
    }

    CameraSample sample(const Point2 &normalized, Sampler &rng) const override {
        auto ray_origin_screen =
            Ray(Vector(0.f, 0.f, 0.f),
                Vector(normalized.x(), normalized.y() * aspect_ratio_inv, z))
                .normalized();
        auto t = focal_distance / ray_origin_screen.direction.z();
        Point p_in_focus_plane = ray_origin_screen(t);
        Point2 sample_2d = rng.next2D();
        Point lens_sample(
            sample_2d.x() * lens_radius, sample_2d.y() * lens_radius, 0);
        auto direction = p_in_focus_plane - lens_sample;
        auto ray_from_focus_plane =
            Ray(p_in_focus_plane, direction).normalized();
        t = -focal_distance / ray_from_focus_plane.direction.z();
        Point ins_origin_plane = ray_from_focus_plane(t);
        auto ray_cam = Ray(ins_origin_plane, direction);
        auto ray_world = m_transform->apply(ray_cam).normalized();
        return CameraSample{.ray = ray_world, .weight = Color(1.0f)};
    }

    std::string toString() const override {
        return tfm::format(
            "ThinLens[\n"
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
    float lens_radius, focal_distance;
};

}

REGISTER_CAMERA(ThinLens, "thinLens")
