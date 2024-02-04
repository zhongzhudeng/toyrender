#include "lightwave/properties.hpp"
#include "lightwave/registry.hpp"
#include "lightwave/sampler.hpp"
#include "lightwave/shape.hpp"

namespace lightwave {

inline Vector sphericalDirection(float sinTheta, float cosTheta, float phi) {
    return Vector(std::clamp(sinTheta, -1.f, 1.f) * std::cos(phi),
                  std::clamp(sinTheta, -1.f, 1.f) * std::sin(phi),
                  std::clamp(cosTheta, -1.f, 1.f));
}

inline float sphericalTheta(const Vector &w) { return safe_acos(w.y()); }

inline float sphericalPhi(const Vector &w) { return std::atan2(w.x(), w.z()); }

inline Point2 to_uv(const Vector &w) {
    float u = sphericalPhi(w) * Inv2Pi + 0.5;
    float v = sphericalTheta(w) * InvPi;
    return Point2(u, v);
}

inline Vector to_cartisian(Point2 &uv) {
    float theta = uv.y() * Pi;
    float sinTheta = std::sin(theta);
    float cosTheta = std::cos(theta);
    float phi = uv.x() * 2 * Pi;
    return sphericalDirection(sinTheta, cosTheta, phi);
}

class Sphere final : public Shape {

public:
    Sphere(const Properties &properties) {}

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        auto dc = Vector(ray.origin);
        auto dc_len2 = dc.lengthSquared();
        auto [dc_len, dcn] = dc.lengthAndNormalized();

        if (std::abs(dc_len - 1.f) < Epsilon && dc.dot(ray.direction) >= 0)
            return false;

        float b = 2 * dc.dot(ray.direction);

        float delta = sqr(b) - 4 * dc_len2 + 4;
        if (safe_sqrt(delta) <= Epsilon) [[unlikely]]
            return false;

        float t = (-b - std::sqrt(delta)) / 2;
        if (t < Epsilon) [[unlikely]] {
            t = (-b + std::sqrt(delta)) / 2;
            if (t < Epsilon) [[unlikely]]
                return false;
        }
        if (its.t < t) [[unlikely]]
            return false;
        its.t = t;
        Vector w = Vector(ray(t)).normalized();
        its.position = w;
        its.frame = Frame(w);
        its.uv = to_uv(w);

        float sinThetaMax = 1.f / dc_len;
        float sin2ThetaMax = sqr(sinThetaMax);
        float oneMinusCosThetaMax;
        if (sin2ThetaMax < 0.00068523f) {
            oneMinusCosThetaMax = sin2ThetaMax / 2;
        } else {
            float cosThetaMax = safe_sqrt(1 - sin2ThetaMax);
            oneMinusCosThetaMax = 1 - cosThetaMax;
        }
        its.pdf = Inv2Pi / ((dc - w).lengthSquared() * oneMinusCosThetaMax);

        return true;
    }

    Bounds getBoundingBox() const override {
        return Bounds(Point(-1, -1, -1), Point(+1, +1, +1));
    }

    Point getCentroid() const override { return Point(0); }

    AreaSample sampleArea(const Point &origin, Sampler &rng) const override {
        AreaSample as;
        Vector dc = Vector(origin);
        Vector dcn = dc.normalized();

        Point2 u = rng.next2D();

        float len = dc.length();
        float sinThetaMax = 1.f / len;
        float sin2ThetaMax = sqr(sinThetaMax);
        float cosThetaMax, oneMinusCosThetaMax, cosTheta, sin2Theta;
        if (sin2ThetaMax < 0.00068523f) [[unlikely]] {
            sin2Theta = sin2ThetaMax * u[0];
            cosTheta = std::sqrt(1 - sin2Theta);
            oneMinusCosThetaMax = sin2ThetaMax / 2;
        } else {
            cosThetaMax = safe_sqrt(1 - sin2ThetaMax);
            oneMinusCosThetaMax = 1 - cosThetaMax;
            cosTheta = (cosThetaMax - 1) * u[0] + 1;
            sin2Theta = 1 - sqr(cosTheta);
        }
        float cosAlpha = sin2Theta / sinThetaMax +
                         cosTheta * safe_sqrt(1 - sin2Theta / sqr(sinThetaMax));
        float sinAlpha = safe_sqrt(1 - sqr(cosAlpha));
        float phi = u[1] * 2 * Pi;
        Vector w =
            Frame(dcn).toWorld(sphericalDirection(sinAlpha, cosAlpha, phi));
        Vector xxp = dc - w;
        as.pdf = Inv2Pi / (xxp.lengthSquared() * oneMinusCosThetaMax);
        as.frame = Frame(w);
        as.position = w;
        as.uv = to_uv(w);
        return as;
    }

    std::string toString() const override { return "Sphere[]"; }
};
}
REGISTER_SHAPE(Sphere, "sphere")