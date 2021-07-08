#ifndef LANTERN_DIMLY_LIT_MAZE_HPP
#define LANTERN_DIMLY_LIT_MAZE_HPP
#include <array>
#include <optional>

#include <maze/distance.hpp>
#include <maze/generate_recursive_backtracking.hpp>
#include <maze/maze.hpp>
#include <termox/termox.hpp>

#include "palette.hpp"

namespace lantern {

template <maze::Distance Width, maze::Distance Height, ox::Color lantern_color>
class Dimly_lit_maze : public ox::Widget {
   public:
    static constexpr auto width  = Width;
    static constexpr auto height = Height;

   public:
    /// Emitted when wanderer makes it to the end of the maze.
    sl::Signal<void()> maze_complete;
    // TODO reveal the whole map then wait a second or two and change mazes.
    // do this from the outside by calling a public function that is defined
    // here to show entire maze, the animation happens at the higher level, it
    // sets a timer event, which will wait a second and then the timer event
    // will stop the animation and switch to the next maze.

    /// Emitted when the wanderer goes over the max step count and is reset.
    sl::Signal<void()> time_warp;

   public:
    Dimly_lit_maze()
    {
        using namespace ox::pipe;
        *this | strong_focus() | fixed_width(Width) | fixed_height(Height) |
            on_resize([this](auto area, auto) {
                too_small_ = area.width < Width || area.height < Height;
            });
    }

   public:
    /// Move the wanderer back to the beginning of the maze and reset step count
    void reset()
    {
        step_count_     = 0;
        wanderer_point_ = maze_.start();
        this->update();
    }

    void generate(maze::Point start)
    {
        maze_ = maze::generate_recursive_backtracking<Width, Height>(start);
        wanderer_point_ = maze_.start();
        max_steps_      = maze::longest_path(maze_, maze_.start()).size() - 1;
        step_count_     = 0;
    }

    /// Return the end of the currently set maze.
    [[nodiscard]] auto maze_end() const -> maze::Point { return maze_.end(); }

   protected:
    /// Move wanderer, possibly reset or emit signal.
    auto key_press_event(ox::Key k) -> bool override
    {
        constexpr auto to_direction =
            [](ox::Key k) -> std::optional<maze::Direction> {
            switch (k) {
                using ox::Key;
                case Key::Arrow_up: return maze::Direction::North;
                case Key::Arrow_down: return maze::Direction::South;
                case Key::Arrow_right: return maze::Direction::East;
                case Key::Arrow_left: return maze::Direction::West;
                default: return std::nullopt;
            }
        };

        if (k == ox::Key::r) {
            this->reset();
            time_warp.emit();
        }
        auto const direction = to_direction(k);
        if (direction.has_value()) {
            auto const next = maze::utility::next_point<Width, Height>(
                wanderer_point_, *direction);
            if (next.has_value() && maze_.get(*next) == maze::Cell::Passage) {
                wanderer_point_ = *next;
                this->update();
                ++step_count_;
                if (wanderer_point_ == maze_.end())
                    maze_complete.emit();
                else if (step_count_ == max_steps_) {
                    this->reset();
                    time_warp.emit();
                }
            }
        }
        return Widget::key_press_event(k);
    }

    //     maze coords match to this widget coords
    auto paint_event(ox::Painter& p) -> bool override
    {
        constexpr auto wanderer_glyph =
            U'🯅' | bg(color::Lantern) | fg(color::Black);

        constexpr auto start_glyph =
            U's' | bg(color::Start) | fg(color::Almost_black);

        constexpr auto end_glyph =
            U'e' | bg(color::End) | fg(color::Almost_black);

        constexpr auto blocks =
            std::array{U'▓' | fg(color::Lantern), U'▒' | fg(color::Lantern),
                       U'░' | fg(color::Lantern)};

        if (too_small_) {
            p.put(U"Screen" | fg(ox::Color::Red), {0, 0});
            p.put(U"Too" | fg(ox::Color::Red), {0, 1});
            p.put(U"Small" | fg(ox::Color::Red), {0, 2});
            return Widget::paint_event(p);
        }

        auto const at = ox::Point{wanderer_point_.x, wanderer_point_.y};
        p.put(wanderer_glyph, at);

        paint_adjacent_walls(p, wanderer_point_);
        for (auto const direction : maze::utility::directions) {
            auto next = std::optional{wanderer_point_};
            for (auto const block : blocks) {
                next =
                    maze::utility::next_point<Width, Height>(*next, direction);
                if (next.has_value() &&
                    maze_.get(*next) == maze::Cell::Passage) {
                    if (*next == maze_.end())
                        p.put(end_glyph, {next->x, next->y});
                    else
                        p.put(block, {next->x, next->y});
                    paint_adjacent_walls(p, *next);
                }
                else
                    break;
            }
        }

        if (wanderer_point_ != maze_.start())
            p.put(start_glyph, {maze_.start().x, maze_.start().y});

        return Widget::paint_event(p);
    }

   private:
    maze::Maze<Width, Height> maze_{maze::Cell::Wall};
    maze::Point wanderer_point_ = {0, 0};
    int max_steps_              = 0;
    int step_count_             = 0;
    bool too_small_             = true;

   private:
    /// Paints up to 8 adjacent wall spaces.
    void paint_adjacent_walls(ox::Painter& p, maze::Point at)
    {
        constexpr auto wall_glyphs = std::array{
            U'🬀', U'🬁', U'🬂', U'🬃', U'🬄', U'🬅', U'🬆',
            U'🬇', U'🬈', U'🬉', U'🬊', U'🬋', U'🬌', U'🬍',
            U'🬎', U'🬏', U'🬐', U'🬑', U'🬒', U'🬓', U'🬔',
            U'🬕', U'🬖', U'🬗', U'🬘', U'🬙', U'🬚', U'🬛',
            U'🬜', U'🬝', U'🬞', U'🬟', U'🬠', U'🬡', U'🬢',
            U'🬣', U'🬤', U'🬥', U'🬦', U'🬧', U'🬨', U'🬩',
            U'🬪', U'🬫', U'🬬', U'🬭', U'🬮', U'🬯', U'🬰',
            U'🬱', U'🬲', U'🬳', U'🬴', U'🬵', U'🬶', U'🬷',
            U'🬸', U'🬹', U'🬺', U'🬻'};

        /// Return the point if it is a Wall, nullopt otherwise.
        auto const get_wall_or_null =
            [this](maze::Point at,
                   maze::Direction direction) -> std::optional<ox::Point> {
            auto pt = maze::utility::next_point<Width, Height>(at, direction);
            if (pt.has_value() && maze_.get(*pt) == maze::Cell::Wall)
                return std::optional{ox::Point{pt->x, pt->y}};
            else
                return std::nullopt;
        };

        auto const north = get_wall_or_null(at, maze::Direction::North);
        auto const south = get_wall_or_null(at, maze::Direction::South);
        auto const east  = get_wall_or_null(at, maze::Direction::East);
        auto const west  = get_wall_or_null(at, maze::Direction::West);

        /// Find the wall glyph associated with the given Point.
        auto const find_wall = [&](ox::Point p) {
            return wall_glyphs[(p.x + p.y) % wall_glyphs.size()] |
                   fg(color::Wall_1) | bg(color::Wall_2);
        };

        if (north.has_value())
            p.put(find_wall(*north), *north);
        if (south.has_value())
            p.put(find_wall(*south), *south);
        if (east.has_value())
            p.put(find_wall(*east), *east);
        if (west.has_value())
            p.put(find_wall(*west), *west);

        if (north.has_value() && west.has_value())
            p.put(find_wall({west->x, north->y}), {west->x, north->y});
        if (north.has_value() && east.has_value())
            p.put(find_wall({east->x, north->y}), {east->x, north->y});
        if (south.has_value() && west.has_value())
            p.put(find_wall({west->x, south->y}), {west->x, south->y});
        if (south.has_value() && east.has_value())
            p.put(find_wall({east->x, south->y}), {east->x, south->y});
    }
};

}  // namespace lantern
#endif  // LANTERN_DIMLY_LIT_MAZE_HPP
