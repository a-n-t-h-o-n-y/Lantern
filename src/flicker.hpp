#ifndef LANTERN_FLICKER_HPP
#define LANTERN_FLICKER_HPP
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>

#include <termox/painter/dynamic_colors.hpp>

namespace lantern {

class Flicker : private ox::dynamic::Modulation_base {
   public:
    /// Resolution is the number of steps to complete a full cycle.
    /** Offset is the step to start at. */
    explicit Flicker(unsigned resolution, unsigned offset = 0)
        : Modulation_base{resolution, offset}
    {}

   public:
    /// Returns value in range [0.0, 1.0]
    [[nodiscard]] auto operator()() -> double
    {
        // TODO flicker rapid movement.
        constexpr auto pi = 3.1416;
        auto const xpi    = this->get_next_ratio() * pi;

        return std::abs(std::sin(xpi * 4.) + std::sin(xpi * 2.) +
                        std::sin(xpi * 7.)) *
               0.36;

        // return .5 * (1 + std::sin(2 * pi * (this->get_next_ratio() - .25)));
    }
};

/// Creates a flickering light dynamic color centered around HSL \p center.
[[nodiscard]] inline auto flicker(
    ox::HSL const center,
    unsigned const resolution                  = 150,
    unsigned const offset                      = 0,
    std::uint8_t const l_range                 = 5,
    ox::Dynamic_color::Period_t const interval = std::chrono::milliseconds{15})
    -> ox::Dynamic_color
{
    auto const hi = [&] {
        auto x      = center;
        x.lightness = std::min(x.lightness + l_range, 100);
        return x;
    }();
    auto const lo = [&] {
        auto x      = center;
        x.lightness = (x.lightness < l_range) ? 0 : (x.lightness - l_range);
        return x;
    }();
    return {interval, ox::dynamic::Fade{hi, lo, Flicker{resolution, offset}}};
}

}  // namespace lantern
#endif  // LANTERN_FLICKER_HPP
