#ifndef LANTERN_PALETTE_HPP
#define LANTERN_PALETTE_HPP
#include <termox/painter/color.hpp>

namespace lantern::color {

inline constexpr auto Black = ox::Color::Background;
// ...
inline constexpr auto White = ox::Color::Foreground;

}  // namespace lantern::color

namespace lantern {

inline auto const palette = ox::Palette{{
    {color::Black, ox::RGB{0, 0, 0}},
    {color::White, ox::RGB{255, 255, 255}},
}};

}
inline auto const palette = ox::Palette{};
#endif  // LANTERN_PALETTE_HPP
