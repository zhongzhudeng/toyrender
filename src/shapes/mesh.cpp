#include <algorithm>
#include <lightwave.hpp>

#include "../core/plyparser.hpp"
#include "accel.hpp"
#include "lightwave/math.hpp"

namespace lightwave {

/**
 * @brief A shape consisting of many (potentially millions) of triangles, which share an index and vertex buffer.
 * Since individual triangles are rarely needed (and would pose an excessive amount of overhead), collections of
 * triangles are combined in a single shape.
 */
class TriangleMesh : public AccelerationStructure {
    /**
     * @brief The index buffer of the triangles.
     * The n-th element corresponds to the n-th triangle, and each component of the element corresponds to one
     * vertex index (into @c m_vertices ) of the triangle.
     * This list will always contain as many elements as there are triangles.
     */
    std::vector<Vector3i> m_triangles;
    /**
     * @brief The vertex buffer of the triangles, indexed by m_triangles.
     * Note that multiple triangles can share vertices, hence there can also be fewer than @code 3 * numTriangles @endcode
     * vertices.
     */
    std::vector<Vertex> m_vertices;
    /// @brief The file this mesh was loaded from, for logging and debugging purposes.
    std::filesystem::path m_originalPath;
    /// @brief Whether to interpolate the normals from m_vertices, or report the geometric normal instead.
    bool m_smoothNormals;

protected:
    int numberOfPrimitives() const override { return int(m_triangles.size()); }

    bool intersect(int primitiveIndex, const Ray &ray, Intersection &its,
                   Sampler &rng) const override {
        auto vi = m_triangles[primitiveIndex];
        auto vert0 = m_vertices[vi[0]], vert1 = m_vertices[vi[1]],
             vert2 = m_vertices[vi[2]];
        auto edge1 = vert1.position - vert0.position,
             edge2 = vert2.position - vert0.position;

        auto pvec = ray.direction.cross(edge2);
        auto det = edge1.dot(pvec);

        if (det > 1e-8 && det < 1e-8)
            return false;
        auto inv_det = 1.f / det;

        auto tvec = ray.origin - vert0.position;

        auto u = tvec.dot(pvec) * inv_det;
        if (u < 0.f || u > 1.f)
            return false;

        auto qvec = tvec.cross(edge1);
        auto v = ray.direction.dot(qvec) * inv_det;
        if (v < 0.f || u + v > 1.f)
            return 0;

        auto t = edge2.dot(qvec) * inv_det;

        if (t < Epsilon || its.t < t)
            return false;

        its.t = t;
        auto itp = Vertex::interpolate(Vector2(u, v), vert0, vert1, vert2);
        its.uv = itp.texcoords;
        if (m_smoothNormals) {
            its.frame.normal = itp.normal.normalized();
            its.position = itp.position;
        } else {
            its.frame.normal = edge1.cross(edge2).normalized();
            its.position = ray(t);
        }
        its.frame = Frame(its.frame.normal);
        // its.frame.tangent = edge1.normalized();
        // its.frame.bitangent =
        //     its.frame.normal.cross(its.frame.tangent).normalized();

        return true;
    }

    Bounds getBoundingBox(int primitiveIndex) const override {
        auto vi = m_triangles[primitiveIndex];
        auto v_0 = m_vertices[vi[0]].position, v_1 = m_vertices[vi[1]].position,
             v_2 = m_vertices[vi[2]].position;
        Point min, max;
        for (int i = 0; i < 3; i++) {
            min[i] = std::min({v_0[i], v_1[i], v_2[i]});
            max[i] = std::max({v_0[i], v_1[i], v_2[i]});
        }
        return Bounds(min, max);
    }

    Point getCentroid(int primitiveIndex) const override {
        auto vi = m_triangles[primitiveIndex];
        auto v_0 = m_vertices[vi[0]].position, v_1 = m_vertices[vi[1]].position,
             v_2 = m_vertices[vi[2]].position;
        Point centroid;
        for (int i = 0; i < 3; i++) {
            centroid[i] = (v_0[i] + v_1[i] + v_2[i]) / 3;
        }
        return centroid;
    }

public:
    TriangleMesh(const Properties &properties) {
        m_originalPath = properties.get<std::filesystem::path>("filename");
        m_smoothNormals = properties.get<bool>("smooth", true);
        readPLY(m_originalPath.string(), m_triangles, m_vertices);
        logger(EInfo,
               "loaded ply with %d triangles, %d vertices",
               m_triangles.size(),
               m_vertices.size());
        buildAccelerationStructure();
    }

    AreaSample sampleArea(Sampler &rng) const override{
        // only implement this if you need triangle mesh area light sampling for your rendering competition
        NOT_IMPLEMENTED}

    std::string toString() const override {
        return tfm::format(
            "Mesh[\n"
            "  vertices = %d,\n"
            "  triangles = %d,\n"
            "  filename = \"%s\"\n"
            "]",
            m_vertices.size(),
            m_triangles.size(),
            m_originalPath.generic_string());
    }
};

}

REGISTER_SHAPE(TriangleMesh, "mesh")
