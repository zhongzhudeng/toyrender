#include "lightwave/properties.hpp"
#include "lightwave/registry.hpp"
#include "lightwave/texture.hpp"

namespace lightwave {

class ConstantTexture final : public Texture {
    Color m_value;

public:
    ConstantTexture(const Properties &properties) {
        m_value = properties.get<Color>("value");
    }

    Color evaluate(const Point2 &uv) const override { return m_value; }

    float scalar(const Point2 &uv) const override {
        return m_value.luminance();
    }

    ScalarImage scalar() const override {
        Point2i resolution(1, 1);
        std::vector<float> data = {m_value.luminance()};
        return ScalarImage(data, resolution);
    }

    std::string toString() const override {
        return tfm::format(
            "ConstantTexture[\n"
            "  value = %s\n"
            "]",
            indent(m_value));
    }
};

} // namespace lightwave

REGISTER_TEXTURE(ConstantTexture, "constant")
