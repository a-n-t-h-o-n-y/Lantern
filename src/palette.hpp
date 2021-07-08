#ifndef LANTERN_PALETTE_HPP
#define LANTERN_PALETTE_HPP
#include <termox/painter/color.hpp>

#include "flicker.hpp"

namespace lantern::color {

inline constexpr auto Black                     = ox::Color::Background;  // 0
inline constexpr auto Lantern                   = ox::Color{1};
inline constexpr auto Wall_1                    = ox::Color{2};
inline constexpr auto Wall_2                    = ox::Color{3};
inline constexpr auto Start                     = ox::Color{4};
inline constexpr auto End                       = ox::Color{5};
inline constexpr auto About_btn_bg              = ox::Color{6};
inline constexpr auto Back_btn_bg               = ox::Color{7};
inline constexpr auto Reset_maze_btn_bg         = ox::Color{8};
inline constexpr auto Reset_maze_btn_confirm_bg = ox::Color{9};
inline constexpr auto Reset_notify_bg           = ox::Color{10};
inline constexpr auto Almost_black              = ox::Color{11};
inline constexpr auto Attempts_bg               = ox::Color{12};
inline constexpr auto Almost_white              = ox::Color{13};
inline constexpr auto White                     = ox::Color::Foreground;  // 15

}  // namespace lantern::color

namespace lantern {

inline auto const palette = ox::Palette{{
    {color::Black, ox::RGB{0, 0, 0}},
    {color::Lantern, flicker(ox::HSL{50, 65, 40}, 150, 0, 5)},

    // TODO leave these out of base_palette and add them for each level, in an
    // array, and use the active page index to find the palette to set on maze
    // finished, after changing the stack page.
    {color::Wall_1, flicker(ox::HSL{7, 70, 14}, 150, 0, 2)},
    {color::Wall_2, flicker(ox::HSL{234, 3, 3}, 150, 0, 1)},

    {color::Start, ox::RGB{69, 117, 143}},
    {color::End, ox::RGB{143, 69, 127}},
    {color::About_btn_bg, ox::RGB{108, 191, 96}},
    {color::Back_btn_bg, ox::RGB{96, 191, 171}},
    {color::Reset_maze_btn_bg, ox::RGB{191, 185, 96}},
    {color::Reset_maze_btn_confirm_bg, ox::RGB{191, 96, 96}},
    {color::Reset_notify_bg, ox::RGB{230, 57, 57}},
    {color::Almost_black, ox::RGB{10, 10, 10}},
    {color::Attempts_bg, ox::RGB{35, 35, 35}},
    {color::Almost_white, ox::RGB{200, 200, 200}},

    {color::White, ox::RGB{255, 255, 255}},
}};

}
inline auto const palette = ox::Palette{};
#endif  // LANTERN_PALETTE_HPP
