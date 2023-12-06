#include <lightwave.hpp>

namespace lightwave {

class ImageTexture final: public Texture {
    enum class BorderMode {
        Clamp,
        Repeat,
    };

    enum class FilterMode {
        Nearest,
        Bilinear,
    };

    ref<Image> m_image;
    float m_exposure;
    BorderMode m_border;
    FilterMode m_filter;

    static inline Color lerp(Color v0, Color v1, float x) {
        Color res;
        for (int i = 0; i < 3; i++)
            res[i] = std::lerp(v0[i], v1[i], x);
        return res;
    }

public:
    ImageTexture(const Properties &properties) {
        if (properties.has("filename")) {
            m_image = std::make_shared<Image>(properties);
        } else {
            m_image = properties.getChild<Image>();
        }
        m_exposure = properties.get<float>("exposure", 1);

        m_border =
            properties.getEnum<BorderMode>("border",
                                           BorderMode::Repeat,
                                           {
                                               {"clamp", BorderMode::Clamp},
                                               {"repeat", BorderMode::Repeat},
                                           });

        m_filter = properties.getEnum<FilterMode>(
            "filter",
            FilterMode::Bilinear,
            {
                {"nearest", FilterMode::Nearest},
                {"bilinear", FilterMode::Bilinear},
            });
    }

    Color evaluate(const Point2 &uv) const override {
        Point2i resolution = m_image->resolution();
        Point2 m_uv;
        Point2 m_point;
        Point2i pixels[4];
        Color colors[4], u0, u1;
        double s, t;
        m_uv.x() = uv.x() - floor(uv.x());
        m_uv.y() = 1 - (uv.y() - floor(uv.y()));

        if (m_border == BorderMode::Clamp) {
            m_point.x() = std::clamp(
                (int)(uv.x() * resolution.x()), 0, resolution.x() - 1);
            m_point.y() = std::clamp(
                (int)((1 - uv.y()) * resolution.y()), 0, resolution.y() - 1);
        } else {
            m_point.x() = m_uv.x() * resolution.x();
            m_point.y() = m_uv.y() * resolution.y();
        }

        if (m_filter == FilterMode::Nearest) {
            pixels[3].x() = std::min((int)m_point.x(), resolution.x() - 1);
            pixels[3].y() = std::min((int)m_point.y(), resolution.y() - 1);
            return m_exposure * (*m_image)(pixels[3]);
        }

        if (m_border == BorderMode::Clamp) {
            m_point.x() = uv.x() * resolution.x();
            m_point.y() = (1 - uv.y()) * resolution.y();
        }

        pixels[3].x() = round(m_point.x());
        pixels[3].y() = round(m_point.y());
        pixels[0].x() = pixels[3].x() - 1;
        pixels[0].y() = pixels[3].y() - 1;
        pixels[1].x() = pixels[3].x() - 1;
        pixels[1].y() = pixels[3].y();
        pixels[2].x() = pixels[3].x();
        pixels[2].y() = pixels[3].y() - 1;

        for (int i = 0; i < 4; i++) {
            if (m_border == BorderMode::Clamp) {
                pixels[i].x() =
                    std::clamp(pixels[i].x(), 0, resolution.x() - 1);
                pixels[i].y() =
                    std::clamp(pixels[i].y(), 0, resolution.y() - 1);
            } else {
                pixels[i].x() =
                    (pixels[i].x() + resolution.x()) % resolution.x();
                pixels[i].y() =
                    (pixels[i].y() + resolution.y()) % resolution.y();
            }
            colors[i] = (*m_image)(pixels[i]);
        }

        s = (m_uv.x() * resolution.x() - round(m_uv.x() * resolution.x())) +
            0.5;
        t = (m_uv.y() * resolution.y() - round(m_uv.y() * resolution.y())) +
            0.5;
        u0 = lerp(colors[0], colors[2], s);
        u1 = lerp(colors[1], colors[3], s);
        return m_exposure * lerp(u0, u1, t);
    }

    std::string toString() const override {
        return tfm::format(
            "ImageTexture[\n"
            "  image = %s,\n"
            "  exposure = %f,\n"
            "]",
            indent(m_image),
            m_exposure);
    }
};

} // namespace lightwave

REGISTER_TEXTURE(ImageTexture, "image")
