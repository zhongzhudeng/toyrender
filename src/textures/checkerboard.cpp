#include <lightwave.hpp>

namespace lightwave {

class CheckerBoardTexture : public Texture {
    Point2 m_scale;
    Color m_color_0;
    Color m_color_1;

public:
    CheckerBoardTexture(const Properties &properties) {
        m_scale = properties.get<Point2>("scale");
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
