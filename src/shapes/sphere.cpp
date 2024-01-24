#include <lightwave.hpp>
namespace lightwave {
class Sphere final : public Shape {
    inline void populate(SurfaceEvent &surf, const Point &position) const {
        auto normal = Vector(position).normalized();
        surf.position = normal;
        surf.frame = Frame(normal);
        surf.uv = toUV(surf.position);
        surf.pdf = Inv4Pi;
    }

public:
    Sphere(const Properties &properties) {}

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {

        auto o_r = Vector(ray.origin);
        if (std::abs(o_r.length() - 1.f) < Epsilon &&
            o_r.dot(ray.direction) >= 0)
            return false;

        float b = 2 * o_r.dot(ray.direction);

        float delta = sqr(b) - 4 * o_r.lengthSquared() + 4;
        if (delta < -Epsilon) [[unlikely]]
            return false;
        delta = std::max(0.f, delta);

        float t = (-b - std::sqrt(delta)) / 2;
        if (t < Epsilon) [[unlikely]] {
            t = (-b + std::sqrt(delta)) / 2;
            if (t < Epsilon) [[unlikely]]
                return false;
        }
        if (its.t < t) [[unlikely]]
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

    AreaSample sampleArea(Sampler &rng) const override {
        AreaSample sample;
        const Point position = squareToUniformSphere(rng.next2D());
        populate(sample, position);
        return sample;
    }

    std::string toString() const override { return "Sphere[]"; }
};
}
REGISTER_SHAPE(Sphere, "sphere")