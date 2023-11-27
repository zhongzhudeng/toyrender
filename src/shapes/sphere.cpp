#include <lightwave.hpp>
namespace lightwave {
class Sphere : public Shape {

public:
    Sphere(const Properties &properties) {}

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {

        auto o_r = ray.origin - Point(0.f, 0.f, 0.f);
        if (std::abs(o_r.length() - 1) < Epsilon)
            return false;

        float b = 2 * o_r.dot(ray.direction);

        const float delta = b * b - 4 * o_r.lengthSquared() + 4;
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
        its.frame.normal = its.position - Point(0);
        its.frame = Frame(its.frame.normal);
        float theta = std::acos(std::clamp(its.position.y(), -1.0f, 1.0f));
        float phi = std::acos(
            std::clamp(its.position.x() /
                           std::sqrt(1 - its.position.y() * its.position.y()),
                       -1.0f,
                       1.0f));
        phi = its.position.z() > 0 ? -phi : phi;
        its.uv.x() = phi * Inv2Pi + 0.5;
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
        sample.uv.x() = rnd.x();
        sample.uv.y() = -rnd.y();
        sample.pdf = Inv4Pi;
        return sample;
    }

    std::string toString() const override { return "Sphere[]"; }
};
}
REGISTER_SHAPE(Sphere, "sphere")