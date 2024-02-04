#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {

void Instance::transformFrame(SurfaceEvent &surf) const {
    if (m_normal) {
        Vector normal =
            (2 * m_normal->evaluate(surf.uv) - Color::white()).data();
        normal = surf.frame.toLocal(normal).normalized();
        surf.frame = Frame(normal);
    }
    surf.position = m_transform->apply(surf.position);
    auto t = m_transform->apply(surf.frame.tangent),
         b = m_transform->apply(surf.frame.bitangent);
    surf.pdf = surf.pdf / t.cross(b).length();
    b = m_flipNormal ? -b : b;
    surf.frame = Frame(t.cross(b).normalized());
}

bool Instance::intersect(const Ray &worldRay, Intersection &its,
                         Sampler &rng) const {
    if (!m_transform) {
        // fast path, if no transform is needed
        Ray localRay = worldRay;
        if (m_shape->intersect(localRay, its, rng)) {
            its.instance = this;
            return true;
        } else {
            return false;
        }
    }

    const float previousT = its.t;
    auto localRay = m_transform->inverse(worldRay);
    const float scale = localRay.direction.length();
    localRay = localRay.normalized();

    its.t *= scale;

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);
    if (wasIntersected) {
        its.t /= scale;
        its.instance = this;
        transformFrame(its);
        return true;
    } else {
        its.t = previousT;
        return false;
    }
}

Bounds Instance::getBoundingBox() const {
    if (!m_transform) {
        // fast path
        return m_shape->getBoundingBox();
    }

    const Bounds untransformedAABB = m_shape->getBoundingBox();
    if (untransformedAABB.isUnbounded()) {
        return Bounds::full();
    }

    Bounds result;
    for (int point = 0; point < 8; point++) {
        Point p = untransformedAABB.min();
        for (int dim = 0; dim < p.Dimension; dim++) {
            if ((point >> dim) & 1) {
                p[dim] = untransformedAABB.max()[dim];
            }
        }
        p = m_transform->apply(p);
        result.extend(p);
    }
    return result;
}

Point Instance::getCentroid() const {
    if (!m_transform) {
        // fast path
        return m_shape->getCentroid();
    }

    return m_transform->apply(m_shape->getCentroid());
}

AreaSample Instance::sampleArea(const Point &reference, Sampler &rng) const {
    const Point ref_local = m_transform->inverse(reference);
    AreaSample sample = m_shape->sampleArea(ref_local, rng);
    if (sample.isInvalid())
        return sample;
    transformFrame(sample);
    return sample;
}
}

REGISTER_CLASS(Instance, "instance", "default")
