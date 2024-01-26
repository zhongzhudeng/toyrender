#include <lightwave.hpp>
namespace lightwave {
class Sphere final : public Shape {
    const bool m_improved_sample;

public:
    Sphere(const Properties &properties)
        : m_improved_sample(properties.get<bool>("improvedsample", false)) {}

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {

        auto o_r = Vector(ray.origin);
        if (std::abs(o_r.length() - 1.f) < 1e-4 && o_r.dot(ray.direction) >= 0)
            return false;

        float b = 2 * o_r.dot(ray.direction);

        float delta = sqr(b) - 4 * o_r.lengthSquared() + 4;
        if (delta <= 1e-4) [[unlikely]]
            return false;

        float t = (-b - std::sqrt(delta)) / 2;
        if (t < 1e-4) [[unlikely]] {
            t = (-b + std::sqrt(delta)) / 2;
            if (t < 1e-4) [[unlikely]]
                return false;
        }
        if (its.t < t) [[unlikely]]
            return false;
        its.t = t;
        auto n = Vector(ray(t)).normalized();
        its.position = Point(n);
        its.frame = Frame(n);
        its.uv = toUV(n);
        if (m_improved_sample) {
            float sinThetaMax = 1.f / o_r.length();
            float sin2ThetaMax = sqr(sinThetaMax);
            float oneMinusCosThetaMax;
            if (sin2ThetaMax < 0.00068523f) {
                oneMinusCosThetaMax = sin2ThetaMax / 2;
            } else {
                float cosThetaMax = safe_sqrt(1 - sin2ThetaMax);
                oneMinusCosThetaMax = 1 - cosThetaMax;
            }
            its.pdf = Inv2Pi / oneMinusCosThetaMax;
        } else {
            its.pdf = Inv4Pi;
        }

        return true;
    }

    Bounds getBoundingBox() const override {
        return Bounds(Point(-1, -1, -1), Point(+1, +1, +1));
    }

    Point getCentroid() const override { return Point(0); }

    AreaSample sampleArea(const Point &origin, Sampler &rng) const override {
        AreaSample as;
        if (m_improved_sample) {
            auto [dc, n] = Vector(origin).lengthAndNormalized();
            Frame sampleingFrame(n);
            float sinThetaMax = 1.f / dc;
            float sin2ThetaMax = sqr(sinThetaMax);
            float cosThetaMax = safe_sqrt(1 - sin2ThetaMax);
            float oneMinusCosThetaMax = 1 - cosThetaMax;
            Point2 u = rng.next2D();
            float cosTheta = (cosThetaMax - 1) * u[0] + 1;
            float sin2Theta = 1 - sqr(cosTheta);

            if (sin2ThetaMax < 0.00068523f) {
                sin2Theta = sin2ThetaMax * u[0];
                cosTheta = std::sqrt(1 - sin2Theta);
                oneMinusCosThetaMax = sin2ThetaMax / 2;
            }

            float cosAlpha =
                sin2Theta / sinThetaMax +
                cosTheta * safe_sqrt(1 - sin2Theta / sqr(sinThetaMax));
            float sinAlpha = safe_sqrt(1 - sqr(cosAlpha));
            float phi = u[1] * 2 * Pi;
            Vector w = sphericalDirection(sinAlpha, cosAlpha, phi);
            w = sampleingFrame.toWorld(w).normalized();
            assert(w.dot(n) >= 0);
            as.position = w;
            as.frame = Frame(w);
            as.uv = toUV(as.position);
            as.pdf = Inv2Pi / oneMinusCosThetaMax;
        } else {
            as.position = squareToUniformSphere(rng.next2D());
            as.frame = Frame(Vector(as.position));
            as.uv = toUV(as.position);
            as.pdf = Inv4Pi;
        }
        return as;
    }

    std::string toString() const override { return "Sphere[]"; }
};
}
REGISTER_SHAPE(Sphere, "sphere")