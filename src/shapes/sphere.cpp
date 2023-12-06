#include <lightwave.hpp>
namespace lightwave {
class Sphere final: public Shape {

public:
    Sphere(const Properties &properties) {}

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {

        auto o_r = ray.origin - Point(0);
        if (std::abs(o_r.length() - 1.f) < Epsilon &&
            o_r.dot(ray.direction) >= 0)
            return false;

        float b = 2 * o_r.dot(ray.direction);

        const float delta = sqr(b) - 4 * o_r.lengthSquared() + 4;
        if (delta < Epsilon)
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
        its.position = ray(t);
        its.frame.normal = (its.position - Point(0)).normalized();
        its.frame = Frame(its.frame.normal);
        float theta = std::acos(std::clamp(its.position.y(), -1.0f, 1.0f));
        float phi =
            std::acos(its.position.x() / std::sqrt(1 - sqr(its.position.y())));
        phi = its.position.z() > 0 ? -phi : phi;
        its.uv.x() = phi * Inv2Pi + 0.5f;
        its.uv.y() = theta * InvPi;
        return true;
    }

    Bounds getBoundingBox() const override {
        return Bounds(Point(-1, -1, -1), Point(+1, +1, +1));
    }

    Point getCentroid() const override { return Point(0); }

    AreaSample sampleArea(Sampler &rng) const override {
        Point2 rnd = rng.next2D();
        AreaSample sample;
        sample.position = squareToUniformSphere(rnd);
        sample.frame.normal = sample.position - Point(0);
        sample.frame = Frame(sample.frame.normal);
        sample.uv = rnd;
        sample.pdf = Inv4Pi;
        return sample;
    }

    std::string toString() const override { return "Sphere[]"; }
};
}
REGISTER_SHAPE(Sphere, "sphere")