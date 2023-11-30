#include <lightwave/core.hpp>
#include <lightwave/instance.hpp>
#include <lightwave/registry.hpp>
#include <lightwave/sampler.hpp>

namespace lightwave {

void Instance::transformFrame(SurfaceEvent &surf) const {
    surf.position = m_transform->apply(surf.position);
    if (m_normal) {
        Vector normal = m_normal->evaluate(surf.uv).data();
        normal = (normal - Vector(0.5)) * 2;
        normal.z() -= 1; 
        surf.frame.normal =surf.frame.normal + normal.normalized();
        surf.frame = Frame(surf.frame.normal.normalized());
    }
    surf.frame.tangent = m_transform->apply(surf.frame.tangent).normalized();
    surf.frame.bitangent =
        m_transform->apply(surf.frame.bitangent).normalized();
    if (m_flipNormal) {
        surf.frame.bitangent = -surf.frame.bitangent;
    }

    surf.frame.normal =
        surf.frame.tangent.cross(surf.frame.bitangent).normalized();

    surf.frame.bitangent =
        surf.frame.normal.cross(surf.frame.tangent).normalized();
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
    Ray localRay = m_transform->inverse(worldRay);
    const float scale = localRay.direction.length();
    its.t = its.t * scale;
    localRay = localRay.normalized();

    const bool wasIntersected = m_shape->intersect(localRay, its, rng);
    if (wasIntersected) {
        its.t = its.t / scale;
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

AreaSample Instance::sampleArea(Sampler &rng) const {
    AreaSample sample = m_shape->sampleArea(rng);
    transformFrame(sample);
    return sample;
}

}

REGISTER_CLASS(Instance, "instance", "default")
