#ifndef LANTERN_DIMLY_LIT_MAZE_HPP
#define LANTERN_DIMLY_LIT_MAZE_HPP
#include <algorithm>
#include <array>
#include <optional>
#include <random>

#include <maze/distance.hpp>
#include <maze/generate_kruskal.hpp>
#include <maze/generate_prims.hpp>
#include <maze/generate_recursive_backtracking.hpp>
#include <maze/longest_path.hpp>
#include <maze/maze.hpp>
#include <termox/termox.hpp>

#include "generator.hpp"
#include "palette.hpp"

namespace lantern {

template <maze::Distance Width, maze::Distance Height>
class Dimly_lit_maze : public ox::Widget {
   public:
    static constexpr auto width  = Width;
    static constexpr auto height = Height;

   public:
    /// Emitted when wanderer makes it to the end of the maze.
    sl::Signal<void()> maze_complete;

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
        if (reveal_maze_)
            return;
        step_count_     = 0;
        wanderer_point_ = maze_start_;
        time_warp.emit();
        this->update();
    }

    void generate(Generator type)
    {
        switch (type) {
            case Generator::Recursive_backtracking:
                maze_ = maze::generate_recursive_backtracking<Width, Height>();
                break;
            case Generator::Kruskal:
                maze_ = maze::generate_kruskal<Width, Height>();
                break;
            case Generator::Prims:
                maze_ = maze::generate_prims<Width, Height>();
                break;
        }
        auto const solution = maze::longest_path(maze_);
        assert(!solution.empty());
        max_steps_      = solution.size() - 1;
        maze_start_     = solution.front();
        maze_end_       = solution.back();
        wanderer_point_ = maze_start_;
        step_count_     = 0;
    }

    /// Return the end of the currently set maze.
    [[nodiscard]] auto maze_end() const -> maze::Point { return maze_end_; }

    /// Displays the entire maze when set true, otherwise only shows neighbors.
    void reveal_maze(bool reveal = true)
    {
        reveal_maze_ = reveal;
        this->update();
    }

   protected:
    /// Move wanderer, possibly reset or emit signal.
    auto key_press_event(ox::Key k) -> bool override
    {
        constexpr auto to_direction =
            [](ox::Key k) -> std::optional<maze::Direction> {
            switch (k) {
                using ox::Key;
                case Key::w:
                case Key::k:
                case Key::Arrow_up: return maze::Direction::North;
                case Key::s:
                case Key::j:
                case Key::Arrow_down: return maze::Direction::South;
                case Key::d:
                case Key::l:
                case Key::Arrow_right: return maze::Direction::East;
                case Key::a:
                case Key::h:
                case Key::Arrow_left: return maze::Direction::West;
                default: return std::nullopt;
            }
        };

        if (k == ox::Key::r) {
            this->reset();
            if (!reveal_maze_)
                time_warp.emit();
        }
        if (k == ox::Key::N) {
            maze_complete.emit();
            return Widget::key_press_event(k);
        }
        auto const direction = to_direction(k);
        if (direction.has_value()) {
            auto const next = maze::utility::next_point<Width, Height>(
                wanderer_point_, *direction);
            if (next.has_value() && maze_.get(*next) == maze::Cell::Passage) {
                wanderer_point_ = *next;
                this->update();
                ++step_count_;
                if (wanderer_point_ == maze_end_)
                    maze_complete.emit();
                else if (step_count_ == max_steps_) {
                    this->reset();
                    time_warp.emit();
                }
            }
        }
        return Widget::key_press_event(k);
    }

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

        if (reveal_maze_) {
            paint_entire_maze(p, maze_, start_glyph, end_glyph, maze_start_,
                              maze_end_);
            return Widget::paint_event(p);
        }

        auto const at = ox::Point{wanderer_point_.x, wanderer_point_.y};
        p.put(wanderer_glyph, at);

        paint_adjacent_walls(p, wanderer_point_, maze_);
        for (auto const direction : maze::utility::directions) {
            auto next = std::optional{wanderer_point_};
            for (auto const block : blocks) {
                next =
                    maze::utility::next_point<Width, Height>(*next, direction);
                if (next.has_value() &&
                    maze_.get(*next) == maze::Cell::Passage) {
                    if (*next == maze_end_)
                        p.put(end_glyph, {next->x, next->y});
                    else
                        p.put(block, {next->x, next->y});
                    paint_adjacent_walls(p, *next, maze_);
                }
                else
                    break;
            }
        }

        if (wanderer_point_ != maze_start_)
            p.put(start_glyph, {maze_start_.x, maze_start_.y});

        return Widget::paint_event(p);
    }

   private:
    maze::Maze<Width, Height> maze_{maze::Cell::Wall};
    maze::Point wanderer_point_ = {0, 0};
    maze::Point maze_start_     = {0, 0};
    maze::Point maze_end_       = {0, 0};
    int max_steps_              = 0;
    int step_count_             = 0;
    bool too_small_             = true;
    bool reveal_maze_           = false;

   private:
    /// Find the wall glyph associated with the given Point.
    [[nodiscard]] static auto find_wall(ox::Point p) -> ox::Glyph
    {
        return wall_glyphs_[(p.x + (p.y * Width)) % wall_glyphs_.size()] |
               fg(color::Wall_1) | bg(color::Wall_2);
    }

    [[nodiscard]] static auto get_shuffled_walls() -> std::array<char32_t, 46>
    {
        auto walls = std::array{
            U'🬎', U'🬏', U'🬐', U'🬑', U'🬒', U'🬓', U'🬔',
            U'🬕', U'🬖', U'🬗', U'🬘', U'🬙', U'🬚', U'🬛',
            U'🬜', U'🬝', U'🬞', U'🬟', U'🬠', U'🬡', U'🬢',
            U'🬣', U'🬤', U'🬥', U'🬦', U'🬧', U'🬨', U'🬩',
            U'🬪', U'🬫', U'🬬', U'🬭', U'🬮', U'🬯', U'🬰',
            U'🬱', U'🬲', U'🬳', U'🬴', U'🬵', U'🬶', U'🬷',
            U'🬸', U'🬹', U'🬺', U'🬻'};
        static auto gen = std::mt19937{std::random_device{}()};
        std::shuffle(std::begin(walls), std::end(walls), gen);
        return walls;
    }

    /// Paints up to 8 adjacent wall spaces.
    static void paint_adjacent_walls(ox::Painter& p,
                                     maze::Point at,
                                     maze::Maze<Width, Height> const& m)
    {
        /// Return the point if it is a Wall, nullopt otherwise.
        auto const get_wall_or_null =
            [&](maze::Point at,
                maze::Direction direction) -> std::optional<ox::Point> {
            auto pt = maze::utility::next_point<Width, Height>(at, direction);
            if (pt.has_value() && m.get(*pt) == maze::Cell::Wall)
                return std::optional{ox::Point{pt->x, pt->y}};
            else
                return std::nullopt;
        };

        auto const north = get_wall_or_null(at, maze::Direction::North);
        auto const south = get_wall_or_null(at, maze::Direction::South);
        auto const east  = get_wall_or_null(at, maze::Direction::East);
        auto const west  = get_wall_or_null(at, maze::Direction::West);

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

    static void paint_entire_maze(ox::Painter& p,
                                  maze::Maze<Width, Height> const& m,
                                  ox::Glyph start_glyph,
                                  ox::Glyph end_glyph,
                                  maze::Point start,
                                  maze::Point end)
    {
        for (auto x = 0; x < Width; ++x) {
            for (auto y = 0; y < Height; ++y) {
                if (m.get({(maze::Distance)x, (maze::Distance)y}) ==
                    maze::Cell::Wall) {
                    p.put(find_wall({x, y}), {x, y});
                }
            }
        }

        auto const solution = maze::longest_path_from(m, start);
        for (auto point : solution)
            p.put(U'•' | fg(color::Gray), {point.x, point.y});

        p.put(start_glyph, {start.x, start.y});
        p.put(end_glyph, {end.x, end.y});
    }

   private:
    inline static std::array<char32_t, 46> wall_glyphs_ = get_shuffled_walls();
};

}  // namespace lantern
#endif  // LANTERN_DIMLY_LIT_MAZE_HPP
