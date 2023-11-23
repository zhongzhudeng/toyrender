#include <lightwave.hpp>

namespace lightwave {

class NormalTexture : public Texture {
    enum class BorderMode {
        Clamp,
        Repeat,
    };

    ref<Image> m_image;
    BorderMode m_border;

public:
    NormalTexture(const Properties &properties) {
        if (properties.has("filename")) {
            m_image = std::make_shared<Image>(properties);
        } else {
            m_image = properties.getChild<Image>();
        }

        m_border =
            properties.getEnum<BorderMode>("border",
                                           BorderMode::Repeat,
                                           {
                                               {"clamp", BorderMode::Clamp},
                                               {"repeat", BorderMode::Repeat},
                                           });
    }

    Color evaluate(const Point2 &uv) const override {
        Point2i resolution = m_image->resolution();
        Point2i pixel;

        if (m_border == BorderMode::Clamp) {
            pixel.x() = std::clamp(
                (int)(uv.x() * resolution.x()), 0, resolution.x() - 1);
            pixel.y() = std::clamp(
                (int)((1 - uv.y()) * resolution.y()), 0, resolution.y() - 1);
        } else {
            pixel.x() =
                std::min((int)((uv.x() - floor(uv.x())) * resolution.x()),
                         resolution.x() - 1);
            pixel.y() =
                std::min((int)((1 - (uv.y() - floor(uv.y()))) * resolution.y()),
                         resolution.y() - 1);
        }
        return (*m_image)(pixel);
    }

    std::string toString() const override {
        return tfm::format(
            "NormalTexture[\n"
            "  image = %s,\n"
            "]",
            indent(m_image));
    }
};

} // namespace lightwave

REGISTER_TEXTURE(NormalTexture, "normal")
