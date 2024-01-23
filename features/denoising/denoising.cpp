#include <lightwave.hpp>

namespace lightwave {
class Denoising final : public Postprocess {

public:
    Denoising(const Properties &properties) : Postprocess(properties) {}

    void execute() override {}

    std::string toString() const override {
        return tfm::format(
            "Conductor[\n"
            "  reflectance = \n"
            "]");
    }
};
} // namespace lightwave

REGISTER_POSTPROCESS(Denoising, "denoising")