#include <lightwave.hpp>

namespace lightwave {

struct SphQuad {
    Point o, p0;
    float x1, y1;
    float b0, b1;
    float k, S;

    SphQuad(const Point &origin) {
        o = origin;
        p0 = Point{-1 - o.x(), -1 - o.y(), -o.z()};
        x1 = p0.x() + 2;
        y1 = p0.y() + 2;
        Vector v00 = Vector(p0);
        Vector v01 = {p0.x(), y1, p0.z()};
        Vector v10 = {x1, p0.y(), p0.z()};
        Vector v11 = {x1, y1, p0.z()};

        Vector n0 = v00.cross(v10).normalized();
        Vector n1 = v10.cross(v11).normalized();
        Vector n2 = v11.cross(v01).normalized();
        Vector n3 = v01.cross(v00).normalized();

        float g0 = std::acos(-n0.dot(n1));
        float g1 = std::acos(-n1.dot(n2));
        float g2 = std::acos(-n2.dot(n3));
        float g3 = std::acos(-n3.dot(n0));

        b0 = n0.z();
        b1 = n2.z();
        k = 2 * Pi - g2 - g3;
        S = g0 + g1 - k;
    }

    Point sample(const Point2 &uv) {
        float au = uv.x() * S + k;
        float fu = (std::cos(au) * b0 - b1) / std::sin(au);
        float cu = std::copysign(1.f, fu) / std::sqrt(sqr(fu) + sqr(b0));
        cu = std::clamp(cu, -1.f, 1.f);
        float xu = -(cu * p0.z()) / sqrt(1 - sqr(cu));
        xu = std::clamp(xu, p0.x(), x1);
        float d = sqrt(sqr(xu) + sqr(p0.z()));
        float h0 = p0.y() / sqrt(sqr(d) + sqr(p0.y()));
        float h1 = y1 / sqrt(sqr(d) + sqr(y1));
        float hv = h0 + uv.y() * (h1 - h0);
        float hv2 = sqr(hv);
        float yv = (hv2 < 1 - Epsilon) ? (hv * d) / sqrt(1 - hv2) : y1;
        return {o.x() + xu, o.y() + yv, 0};
    }

    void polulate(SurfaceEvent &surf, const Point &position) {
        surf.uv = {(position.x() + 1) / 2, (position.y() + 1) / 2};
        surf.position = {position.x(), position.y(), 0};
        surf.frame.normal = Vector(0, 0, 1);
        surf.frame.tangent = Vector(1, 0, 0);
        surf.frame.bitangent = Vector(0, 1, 0);
        surf.pdf = 1.f / (S * (surf.position - o).lengthSquared());
    }
};

/// @brief A rectangle in the xy-plane, spanning from [-1,-1,0] to [+1,+1,0].
class Rectangle final : public Shape {

public:
    Rectangle(const Properties &properties) {}

    bool intersect(const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        // if the ray travels in the xy-plane, we report no intersection
        // (we ignore the edge case - pun intended - that the ray might have infinite intersections with the rectangle)
        if (ray.direction.z() == 0) [[unlikely]]
            return false;

        // ray.origin.z + t * ray.direction.z = 0
        // <=> t = -ray.origin.z / ray.direction.z
        const float t = -ray.origin.z() / ray.direction.z();

        // note that we never report an intersection closer than Epsilon (to avoid self-intersections)!
        // we also do not update the intersection if a closer intersection already exists (i.e., its.t is lower than our own t)
        if (t < Epsilon || t > its.t)
            return false;

        // compute the hitpoint
        const Point position = ray(t);
        // we have intersected an infinite plane at z=0; now dismiss anything outside of the [-1,-1,0]..[+1,+1,0] domain.
        if (std::abs(position.x()) > 1 || std::abs(position.y()) > 1)
            return false;

        // we have determined there was an intersection! we are now free to change the intersection object and return true.
        its.t = t;
        SphQuad squad(ray.origin);
        squad.polulate(its, position);
        return true;
    }

    Bounds getBoundingBox() const override {
        return Bounds(Point{-1, -1, 0}, Point{+1, +1, 0});
    }

    Point getCentroid() const override { return Point(0); }

    AreaSample sampleArea(const Point &origin, Sampler &rng) const override {
        if (origin.z() <= 0)
            return AreaSample::invalid();
        auto squad = SphQuad(origin);
        Point position = squad.sample(rng.next2D());
        AreaSample sample;
        squad.polulate(sample, position);
        return sample;
    }

    std::string toString() const override { return "Rectangle[]"; }
};

}

// this informs lightwave to use our class Rectangle whenever a <shape type="rectangle" /> is found in a scene file
REGISTER_SHAPE(Rectangle, "rectangle")
