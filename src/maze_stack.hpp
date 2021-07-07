#ifndef LANTERN_MAZE_STACK_HPP
#define LANTERN_MAZE_STACK_HPP
#include <cassert>
#include <cstddef>
#include <map>
#include <utility>

#include <termox/termox.hpp>

#include "dimly_lit_maze.hpp"

namespace lantern {

using Mazes_tuple = ox::STuple<Dimly_lit_maze<10, 5, ox::Color::Yellow>,
                               Dimly_lit_maze<15, 8, ox::Color::Yellow>,
                               Dimly_lit_maze<20, 10, ox::Color::Yellow>,
                               Dimly_lit_maze<25, 13, ox::Color::Yellow>,
                               Dimly_lit_maze<30, 15, ox::Color::Yellow>,
                               Dimly_lit_maze<35, 18, ox::Color::Yellow>,
                               Dimly_lit_maze<40, 20, ox::Color::Yellow>,
                               Dimly_lit_maze<45, 23, ox::Color::Yellow>,
                               Dimly_lit_maze<50, 25, ox::Color::Yellow>>;

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

        this->initialize_mazes(std::make_index_sequence<9>());

        this->set_maze(0);

        maze_reset.connect(
            [this] { this->increment_attempts(this->active_page_index()); });
    }

   public:
    void reset_active_maze()
    {
        auto const index = this->active_page_index();
        this->increment_attempts(index);
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

        // TODO change palette colors?

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
    void initialize_mazes(std::index_sequence<Is...>)
    {
        auto const emit_reset = [this] { this->maze_reset.emit(); };
        (this->get<Is>().time_warp.connect(emit_reset), ...);
        auto const goto_next_maze = [this] { this->increment_maze(); };
        (this->get<Is>().maze_complete.connect(goto_next_maze), ...);
        auto start = maze::utility::random_point<10, 5>();
        ((this->get<Is>().generate(start), start = this->get<Is>().maze_end()),
         ...);
    }
};

class Floating_maze_stack : public ox::Float_2d<Maze_stack> {
   public:
    Floating_maze_stack()
    {
        using namespace ox::pipe;
        constexpr auto background = ox::Color::Dark_gray;
        this->buffer_1 | bg(background);
        this->buffer_2 | bg(background);
        this->widget.buffer_1 | bg(background);
        this->widget.buffer_2 | bg(background);

        *this | direct_focus() | forward_focus(this->widget.widget);
    }
};

}  // namespace lantern
#endif  // LANTERN_MAZE_STACK_HPP
