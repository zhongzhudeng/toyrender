#include <OpenImageDenoise/oidn.hpp>
#include <lightwave.hpp>

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

        /*
        auto res = m_input->resolution();
        auto xy = res.x() * res.y() * 3 * sizeof(float);
        oidn::DeviceRef device = oidn::newDevice();
        oidn::FilterRef filter = device.newFilter("RT");
        oidn::BufferRef colorBuf = device.newBuffer(xy);
        oidn::BufferRef albedoBuf = device.newBuffer(xy);
        oidn::BufferRef normalBuf = device.newBuffer(xy);
        oidn::BufferRef outputBuf = device.newBuffer(xy);

        filter.setImage(
            "color", colorBuf, oidn::Format::Float3, res.x(), res.y());
        filter.setImage(
            "albedo", albedoBuf, oidn::Format::Float3, res.x(), res.y());
        filter.setImage(
            "normal", normalBuf, oidn::Format::Float3, res.x(), res.y());
        filter.setImage(
            "output", outputBuf, oidn::Format::Float3, res.x(), res.y());
        // beauty image is HDR
        filter.set("hdr", true);
        // auxiliary images will be prefiltered
        filter.set("cleanAux", true);
        filter.commit();

        // Create a separate filter for denoising an auxiliary albedo image (in-place)
        // same filter type as for beauty
        oidn::FilterRef albedoFilter = device.newFilter("RT");
        albedoFilter.setImage("albedo",
                              albedoBuf,
                              oidn::Format::Float3,
                              m_albedo->resolution().x(),
                              m_albedo->resolution().y());
        albedoFilter.setImage("output",
                              albedoBuf,
                              oidn::Format::Float3,
                              m_albedo->resolution().x(),
                              m_albedo->resolution().y());
        albedoFilter.commit();

        // Create a separate filter for denoising an auxiliary normal image (in-place)
        // same filter type as for beauty
        oidn::FilterRef normalFilter = device.newFilter("RT");
        normalFilter.setImage("normal",
                              normalBuf,
                              oidn::Format::Float3,
                              m_normals->resolution().x(),
                              m_normals->resolution().y());
        normalFilter.setImage("output",
                              normalBuf,
                              oidn::Format::Float3,
                              m_normals->resolution().x(),
                              m_normals->resolution().y());
        normalFilter.commit();

        // Prefilter the auxiliary images
        float *colorPtr = (float *)colorBuf.getData();
        float *albedoPtr = (float *)albedoBuf.getData();
        float *normalPtr = (float *)normalBuf.getData();
        std::memcpy(colorPtr, m_input->data(), xy);
        std::memcpy(albedoPtr, m_albedo->data(), xy);
        std::memcpy(normalPtr, m_normals->data(), xy);

        albedoFilter.execute();
        normalFilter.execute();

        // Filter the beauty image
        filter.execute();

        m_output->initialize(m_input->resolution());
        float *outputPtr = (float *)outputBuf.getData();
        std::memcpy(m_output->data(), outputPtr, xy);
        Streaming stream{*m_output};
        stream.update();
        m_output->save();
        */
    }

    std::string toString() const override {
        return tfm::format(
            "Denoising[\n"
            "]");
    }
};
} // namespace lightwave

REGISTER_POSTPROCESS(Denoising, "denoising")