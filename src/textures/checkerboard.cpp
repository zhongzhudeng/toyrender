#include "lightwave/properties.hpp"
#include "lightwave/registry.hpp"
#include "lightwave/texture.hpp"

namespace lightwave {

class CheckerBoardTexture final : public Texture {
    Point2i m_scale;
    Color m_color_0;
    Color m_color_1;

public:
    CheckerBoardTexture(const Properties &properties) {
        m_scale = properties.get<Point2i>("scale");
        m_color_0 = properties.get<Color>("color0");
        m_color_1 = properties.get<Color>("color1");
    }

    Color evaluate(const Point2 &uv) const override {
        int x = uv.x() * m_scale.x();
        int y = uv.y() * m_scale.y();

        if ((x + y) % 2 == 0)
            return m_color_0;
        else
            return m_color_1;
    }

    float scalar(const Point2 &uv) const override {
        return evaluate(uv).luminance();
    }

    ScalarImage scalar() const override {
        size_t width = m_scale.x(), height = m_scale.y();
        std::vector<float> data(width * height);
        for (size_t v = 0; v < height; v++) {
            for (size_t u = 0; u < width; u++) {
                if ((u + v) % 2 == 0)
                    data[v * width + u] = m_color_0.luminance();
                else
                    data[v * width + u] = m_color_1.luminance();
            }
        }
        return ScalarImage(data, m_scale);
    }

    std::string toString() const override {
        return tfm::format(
            "CheckerBoardTexture[\n"
            "  scale = %s\n",
            "  color0 = %s\n",
            "  color1 = %s\n"
            "]",
            indent(m_scale),
            indent(m_color_0),
            indent(m_color_1));
    }
};

} // namespace lightwave

REGISTER_TEXTURE(CheckerBoardTexture, "checkerboard")
