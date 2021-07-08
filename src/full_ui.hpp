#ifndef LANTERN_FULL_UI_HPP
#define LANTERN_FULL_UI_HPP
#include <cassert>
#include <map>

#include <termox/termox.hpp>

#include "maze_stack.hpp"
#include "notify_label.hpp"
#include "palette.hpp"

namespace lantern {

class Top_bar : public ox::HLabel {
   public:
    Top_bar() : ox::HLabel{U"", ox::Align::Center}
    {
        using namespace ox::pipe;
        this->set_level_text(1);
        *this | bg(color::Almost_black) | fg(color::Almost_white);
    }

   public:
    void set_level_text(int level)
    {
        assert(level < 10);
        auto message = ox::Glyph_string{U"Level "};
        message.append(std::to_string(level));
        message.append(U" of 9");
        this->ox::HLabel::set_text(std::move(message));
    }

    void display_game_over_text()
    {
        this->ox::HLabel::set_text(U"~ Game Over ~");
    }
};

class Attempts_display
    : public ox::Passive<ox::HPair<ox::HLabel, ox::Int_view>> {
   public:
    ox::HLabel& title        = this->first;
    ox::Int_view& count_view = this->second;

   public:
    Attempts_display()
    {
        using namespace ox::pipe;
        title | text(U"Attempts:") | fixed_width(10);
        title | bg(color::Attempts_bg) | fg(color::Almost_white);
        count_view | bg(color::Attempts_bg) | fg(color::Almost_white) |
            fixed_width(3);
        *this | fixed_width(13);
        this->reset();
    }

   public:
    void set(int value) { count_view.set_value(value); }

    void reset() { count_view.set_value(1); }
};

struct About_btn : ox::Toggle_button {
    About_btn() : ox::Toggle_button{U"About", U"Back"}
    {
        using namespace ox::pipe;
        *this | fixed_width(9);
        top | bg(color::About_btn_bg) | fg(color::Almost_black);
        bottom | bg(color::Back_btn_bg) | fg(color::Almost_black);
    }
};

class Bottom_bar : public ox::HTuple<Attempts_display,
                                     ox::Widget,
                                     About_btn,
                                     ox::Confirm_button,
                                     Notify_label> {
   public:
    Attempts_display& attempts    = this->get<0>();
    Widget& empty_space           = this->get<1>();
    About_btn& about_btn          = this->get<2>();
    ox::Confirm_button& reset_btn = this->get<3>();
    Notify_label& reset_notify    = this->get<4>();

   public:
    Bottom_bar()
    {
        using namespace ox::pipe;
        *this | fixed_height(1);

        empty_space | bg(color::Almost_black);

        reset_btn.main_btn | text(U"Reset Maze") |
            bg(color::Reset_maze_btn_bg) | fg(color::Almost_black);
        reset_btn.confirm_page.confirm_btn |
            bg(color::Reset_maze_btn_confirm_bg) | fg(color::Almost_black);
        reset_btn.confirm_page.exit_btn | bg(color::Almost_black) |
            fg(color::Almost_white);
        reset_btn | fixed_width(12);
        reset_notify | fixed_width(9) | bg(color::Attempts_bg) |
            fg(color::Almost_black);
    }
};

class About_page : public ox::HTuple<ox::Widget, ox::Text_view, ox::Widget> {
   public:
    About_page()
    {
        using namespace ox::pipe;
        this->get<0>() | bg(color::Almost_black);
        this->get<2>() | bg(color::Almost_black);
        text_view | fixed_width(50) | bg(color::Almost_black) |
            fg(color::Almost_white) | text(generate_text());
    }

    //    screen that works on small displays. instructions show way to exit
    //    ctrl-c also how to reset entire game by exiting and restarting.
    //    Explain the idea of getting to the end of the maze in exact number of
    //    steps.

   private:
    ox::Text_view& text_view = this->get<1>();

   private:
    [[nodiscard]] static auto generate_text() -> ox::Glyph_string
    {
        auto result = ox::Glyph_string{U'\n'};
        result.append(U"                      About" | ox::Trait::Bold);
        result.append(U"\n\n");
        result.append(
            U"Travel from the beginning to the end of the maze in the least "
            U"amount of steps possible.\n\nThe wanderer is reset to the start "
            U"if this step count is reached without completing the "
            U"maze.\n\nComplete each level in the least number of "
            U"attempts.\n\n");

        result.append(U"Controls" | ox::Trait::Bold);
        result.append(U"\n• Movement             Arrow Keys / wasd / hjkl");
        result.append(U"\n• Reset Current Level  r");
        result.append(U"\n• Exit App             Ctrl c");
        return result;
    }
};

class End_screen : public ox::HTuple<ox::Widget, ox::Text_view, ox::Widget> {
   public:
    End_screen()
    {
        using namespace ox::pipe;
        text_view | fixed_width(14) | any_wrap() | bg(color::Almost_black) |
            fg(color::Almost_white);
        this->get<0>() | bg(color::Almost_black);
        this->get<2>() | bg(color::Almost_black);
    }

   public:
    void initialize(std::map<int, int> const& attempts)
    {
        text_view.set_text(generate_text(attempts));
    }

   private:
    ox::Text_view& text_view = this->get<1>();

   private:
    [[nodiscard]] static auto generate_text(std::map<int, int> const& attempts)
        -> ox::Glyph_string
    {
        auto result = ox::Glyph_string{U"\n  Attempts\n\n" | ox::Trait::Bold};
        auto total  = 0;
        for (auto i = 0; i < 9; ++i) {
            auto const count = attempts.at(i);
            total += count;
            result.append(U" Maze ").append(std::to_string(i + 1));
            result.append(U": ").append(std::to_string(count)).append('\n');
        }
        result.append(U"\n  Total: ").append(std::to_string(total));
        return result;
    }
};

class Full_UI
    : public ox::VTuple<Top_bar,
                        ox::STuple<Floating_maze_stack, About_page, End_screen>,
                        Bottom_bar> {
   public:
    Top_bar& top_bar       = this->get<0>();
    Maze_stack& maze_stack = this->get<1>().get<0>().widget.widget;
    About_page& about_page = this->get<1>().get<1>();
    End_screen& end_screen = this->get<1>().get<2>();
    Bottom_bar& bottom_bar = this->get<2>();

   public:
    Full_UI()
    {
        using namespace ox::pipe;

        ox::Terminal::set_palette(palette);

        *this | direct_focus() | forward_focus(maze_stack);

        // middle stack - set floating maze stack
        this->get<1>().set_active_page(0);

        maze_stack.maze_reset.connect([this] {
            bottom_bar.reset_notify.set_text(
                U"  Reset  " | bg(color::Reset_notify_bg) | fg(color::Black));
        });

        maze_stack.page_changed.connect(
            [this](auto const index) { top_bar.set_level_text(index + 1); });

        maze_stack.attempts_modified.connect(
            [this](auto count) { bottom_bar.attempts.set(count); });

        maze_stack.game_over.connect([this](auto const& attempts) {
            end_screen.initialize(attempts);
            top_bar.display_game_over_text();
            this->get<1>().set_active_page(2);
        });

        bottom_bar.reset_btn |
            on_press([this] { maze_stack.reset_active_maze(); });

        bottom_bar.about_btn.top |
            on_press([this] { this->get<1>().set_active_page(1); });

        bottom_bar.about_btn.bottom |
            on_press([this] { this->get<1>().set_active_page(0); });
    }
};

}  // namespace lantern
#endif  // LANTERN_FULL_UI_HPP
