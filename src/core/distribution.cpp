#include <lightwave/distribution.hpp>

#include <algorithm>
#include <numeric>
#include <span>
#include <vector>

namespace lightwave {

Distribution1D::Distribution1D() {}

Distribution1D::Distribution1D(std::span<float> data) {
    m_data = data;
    m_size = data.size();
    m_cdf.resize(m_size + 1, 0.f);
    std::transform_inclusive_scan(m_data.begin(),
                                  m_data.end(),
                                  m_cdf.begin() + 1,
                                  std::plus<float>{},
                                  [&](float x) { return x / m_size; });
    m_sum = m_cdf.back();
    if (m_sum == 0)
        for (size_t i = 0; i < m_size + 1; i++)
            m_cdf[i] = float(i) / float(m_size);
    else
        std::for_each(
            m_cdf.begin(), m_cdf.end(), [&](float &v) { v /= m_sum; });
}

std::span<float> Distribution1D::data() const { return m_data; }

Sample1D Distribution1D::sampleContinous(Sampler &rng) const {
    float u = rng.next();
    auto it = std::upper_bound(m_cdf.cbegin(), m_cdf.cend(), u) - 1;
    size_t offset = std::distance(m_cdf.cbegin(), it);
    float du = u - m_cdf[offset];
    if (m_cdf[offset + 1] - m_cdf[offset] > 0)
        du /= (m_cdf[offset + 1] - m_cdf[offset]);
    float pdf = (m_sum == 0) ? 1.f : (m_data[offset] / m_sum);
    // pdf = std::max(1.f, pdf);
    return {
        .index = offset,
        .u = (offset + du) / m_size,
        .pdf = pdf,
    };
}

float Distribution1D::sum() const { return m_sum; }

Distribution2D::Distribution2D() {}

Distribution2D::Distribution2D(ScalarImage scalar) {
    m_resolution = scalar.resolution();
    size_t width = m_resolution.x(), height = m_resolution.y();
    m_marginal_data.reserve(height);

    for (size_t i = 0; i < height; i++) {
        std::span<float> subspan = scalar.data().subspan(i * width, width);
        Distribution1D dist(subspan);
        m_marginal_data.emplace_back(dist.sum());
        m_conditional.emplace_back(dist);
    }

    m_marginal = Distribution1D(m_marginal_data);
}

float Distribution2D::pdf(Point2 uv) const {
    size_t iu =
        std::clamp(int(uv.x() * m_resolution.x()), 0, m_resolution.x() - 1);
    size_t iv =
        std::clamp(int(uv.y() * m_resolution.y()), 0, m_resolution.y() - 1);
    return m_conditional[iv].data()[iu] / m_marginal.sum();
    // return std::max(1.f, m_conditional[iv].data()[iu] / m_marginal.sum());
}

Sample2D Distribution2D::sampleContinous(Sampler &rng) const {
    auto s = m_marginal.sampleContinous(rng);
    auto c = m_conditional[s.index].sampleContinous(rng);
    Point2 uv = {c.u, s.u};
    return {uv, s.pdf * c.pdf};
}

}