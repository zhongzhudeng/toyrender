#include "lightwave/math.hpp"
#include <cmath>
#include <lightwave.hpp>
namespace lightwave {
class Sphere : public Shape {

    inline void populate(SurfaceEvent &surf, const Point &position) const {
        surf.position = position;
        // TODO: uv
        surf.frame.normal = position - Point(0, 0, 0);
        surf.frame.tangent = Vector(0, 1, 0).cross(surf.frame.normal).normalized();
        surf.frame.bitangent = surf.frame.normal.cross(surf.frame.tangent).normalized();
        // TODO: pdf
    }

public:
    Sphere(const Properties &properties) {}

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {

        auto o_r = ray.origin - Point(0.f, 0.f, 0.f);
        float b = 2 * o_r.dot(ray.direction);

        const float delta = b * b - 4 * o_r.lengthSquared() + 4;
        if (delta < 0)
            return false;

        float t = (-b - std::sqrt(delta)) / 2;
        if (t < Epsilon) {
            t = (-b + std::sqrt(delta)) / 2;
            if (t < Epsilon)
                return false;
        }
        if (its.t < t)
            return false;
        its.t = t;
        const Point position = ray(t);
        populate(its, position);
        return true;
    }

    Bounds getBoundingBox() const override {
        return Bounds(Point(-1, -1, -1), Point(+1, +1, +1));
    }

    Point getCentroid() const override { return Point(0); }

    AreaSample sampleArea(Sampler &rng) const override{NOT_IMPLEMENTED}

    std::string toString() const override {
        return "Sphere[]";
    }
};
}
REGISTER_SHAPE(Sphere, "sphere")