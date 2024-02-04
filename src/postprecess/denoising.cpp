#include <OpenImageDenoise/oidn.hpp>

#include "lightwave/image.hpp"
#include "lightwave/postprocess.hpp"
#include "lightwave/registry.hpp"
#include "lightwave/streaming.hpp"

namespace lightwave {
class Denoising final : public Postprocess {
    const ref<Image> m_normals;
    const ref<Image> m_albedo;

public:
    Denoising(const Properties &properties)
        : Postprocess(properties), m_normals(properties.get<Image>("normals")),
          m_albedo(properties.get<Image>("albedo")) {}

    void execute() override {
        auto res = m_input->resolution();
        auto xy = res.x() * res.y() * 3 * sizeof(float);
        oidn::DeviceRef device = oidn::newDevice();
        device.commit();

        oidn::BufferRef colorBuf = device.newBuffer(xy);
        oidn::BufferRef albedoBuf = device.newBuffer(xy);
        oidn::BufferRef normalBuf = device.newBuffer(xy);

        oidn::FilterRef filter = device.newFilter("RT");
        filter.setImage(
            "color", colorBuf, oidn::Format::Float3, res.x(), res.y());
        filter.setImage(
            "albedo", albedoBuf, oidn::Format::Float3, res.x(), res.y());
        filter.setImage(
            "normal", normalBuf, oidn::Format::Float3, res.x(), res.y());
        filter.setImage(
            "output", colorBuf, oidn::Format::Float3, res.x(), res.y());
        filter.set("hdr", true);
        filter.commit();

        float *colorPtr = (float *)colorBuf.getData();
        float *albedoPtr = (float *)albedoBuf.getData();
        float *normalPtr = (float *)normalBuf.getData();
        std::memcpy(colorPtr, m_input->data(), xy);
        std::memcpy(albedoPtr, m_albedo->data(), xy);
        std::memcpy(normalPtr, m_normals->data(), xy);
        filter.execute();
        m_output->initialize(m_input->resolution());
        std::memcpy(m_output->data(), colorPtr, xy);
        Streaming stream{*m_output};
        stream.update();
        m_output->save();
    }

    std::string toString() const override {
        return tfm::format(
            "Denoising[\n"
            "]");
    }
};
} // namespace lightwave

REGISTER_POSTPROCESS(Denoising, "denoising")