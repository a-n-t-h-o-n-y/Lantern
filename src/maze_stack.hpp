#ifndef LANTERN_MAZE_STACK_HPP
#define LANTERN_MAZE_STACK_HPP
#include <cassert>
#include <cstddef>
#include <map>
#include <utility>

#include <termox/termox.hpp>

#include "dimly_lit_maze.hpp"
#include "generator.hpp"
#include "palette.hpp"

namespace lantern {

using Mazes_tuple = ox::STuple<Dimly_lit_maze<11, 7>,
                               Dimly_lit_maze<15, 9>,
                               Dimly_lit_maze<29, 11>,
                               Dimly_lit_maze<35, 13>,
                               Dimly_lit_maze<39, 15>,
                               Dimly_lit_maze<47, 17>,
                               Dimly_lit_maze<53, 19>,
                               Dimly_lit_maze<61, 21>,
                               Dimly_lit_maze<81, 21>>;

/// Holds all levels of generated mazes.
class Maze_stack : public Mazes_tuple {
   public:
    /// Emitted when any maze is reset.
    sl::Signal<void()> maze_reset;

    /// Emitted when all 9 mazes have been complete.
    /** Sends attempts map along, [index, count]. */
    sl::Signal<void(std::map<int, int> const&)> game_over;

    /// Emitted when an attempt has finished, sends the count for current maze.
    sl::Signal<void(int)> attempts_modified;

   public:
    Maze_stack()
    {
        using namespace ox::pipe;
        *this | direct_focus();

        this->set_maze(0);

        maze_reset.connect(
            [this] { this->increment_attempts(this->active_page_index()); });
    }

   public:
    void reset_active_maze()
    {
        auto const index = this->active_page_index();
        switch (index) {
            case 0: this->get<0>().reset(); break;
            case 1: this->get<1>().reset(); break;
            case 2: this->get<2>().reset(); break;
            case 3: this->get<3>().reset(); break;
            case 4: this->get<4>().reset(); break;
            case 5: this->get<5>().reset(); break;
            case 6: this->get<6>().reset(); break;
            case 7: this->get<7>().reset(); break;
            case 8: this->get<8>().reset(); break;
            default: throw std::logic_error{"reset_active_maze bad index."};
        }
    }

    void initialize_mazes(Generator g)
    {
        this->do_initialize_mazes(std::make_index_sequence<9>(), g);
    }

   protected:
    auto timer_event() -> bool override
    {
        this->disable_animation();
        this->increment_maze();
        return Mazes_tuple::timer_event();
    }

   private:
    /// [maze_index, attempt_count]
    std::map<int, int> attempts_ = {{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1},
                                    {5, 1}, {6, 1}, {7, 1}, {8, 1}};

   private:
    void set_maze(int index)
    {
        assert(index < 9);
        this->set_active_page(index);
        attempts_modified.emit(attempts_[index]);

        ox::Terminal::set_palette(palettes[index]);

        auto* maze = this->get_active_page();
        assert(maze != nullptr);
        this->width_policy  = maze->width_policy;
        this->height_policy = maze->height_policy;
    }

    /// Set the next maze as the active page.
    /** Emits game_over if all mazes have been traveled through. */
    void increment_maze()
    {
        auto const next = this->active_page_index() + 1;
        if (next >= this->size())
            game_over.emit(attempts_);
        else
            this->set_maze(next);
    }

    void increment_attempts(int const index)
    {
        assert(index < 9);
        attempts_[index] += 1;
        attempts_modified.emit(attempts_[index]);
    }

    template <std::size_t... Is>
    void do_initialize_mazes(std::index_sequence<Is...>, Generator g)
    {
        auto const emit_reset = [this] { this->maze_reset.emit(); };
        (this->get<Is>().time_warp.connect(emit_reset), ...);
        auto const goto_next_maze = [this](auto index) {
            this->reveal_maze_and_set_timer(index);
        };
        (this->get<Is>().maze_complete.connect([=] { goto_next_maze(Is); }),
         ...);
        ((this->get<Is>().generate(g)), ...);
    }

    void reveal_maze_and_set_timer(std::size_t index)
    {
        switch (index) {
            case 0: this->get<0>().reveal_maze(true); break;
            case 1: this->get<1>().reveal_maze(true); break;
            case 2: this->get<2>().reveal_maze(true); break;
            case 3: this->get<3>().reveal_maze(true); break;
            case 4: this->get<4>().reveal_maze(true); break;
            case 5: this->get<5>().reveal_maze(true); break;
            case 6: this->get<6>().reveal_maze(true); break;
            case 7: this->get<7>().reveal_maze(true); break;
            case 8: this->get<8>().reveal_maze(true); break;
            default: throw std::logic_error{"Invalid Index"};
        }
        this->enable_animation(std::chrono::milliseconds{4'000});
    }
};

class Floating_maze_stack : public ox::Float_2d<Maze_stack> {
   public:
    Floating_maze_stack()
    {
        using namespace ox::pipe;
        constexpr auto background = color::Almost_black;
        this->buffer_1 | bg(background);
        this->buffer_2 | bg(background);
        this->widget.buffer_1 | bg(background);
        this->widget.buffer_2 | bg(background);

        *this | direct_focus() | forward_focus(this->widget.widget);
    }
};

}  // namespace lantern
#endif  // LANTERN_MAZE_STACK_HPP
