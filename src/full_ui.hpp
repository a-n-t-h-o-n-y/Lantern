#ifndef LANTERN_FULL_UI_HPP
#define LANTERN_FULL_UI_HPP
#include <cassert>
#include <map>

#include <termox/termox.hpp>

#include "generator.hpp"
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
        message.append(U" of 9 ");
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

   private:
    ox::Text_view& text_view = this->get<1>();

   private:
    [[nodiscard]] static auto generate_text() -> ox::Glyph_string
    {
        auto result = ox::Glyph_string{U'\n'};
        result.append(U"                     About" | ox::Trait::Bold);
        result.append(U"\n\n");

        result.append(
            U"Complete each maze with minimal visiblity, making no wrong "
            U"turns.\n\n"
            U"The Wanderer is sent back to the beginning whenever the minimum "
            U"number of steps to complete the maze has been reached(without "
            U"making it to the end).\n\n"
            U"Each attempt it tallied\n\n");

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

class Generator_title : public ox::HLabel {
   public:
    Generator_title()
        : ox::HLabel{U"~ Choose Maze Generator ~", ox::Align::Center}
    {
        using namespace ox::pipe;
        *this | fg(color::Almost_white) | bg(color::Almost_black);
    }
};

class Generator_menu : public ox::Menu {
   public:
    sl::Signal<void(Generator)> chosen;

   public:
    Generator_menu()
    {
        using namespace ox::pipe;

        this->append_item(U"Recursive Backtracking").connect([this] {
            chosen.emit(Generator::Recursive_backtracking);
        });
        this->append_item(U"Randomized Kruskal's").connect([this] {
            chosen.emit(Generator::Kruskal);
        });
        this->append_item(U"Randomized Prim's").connect([this] {
            chosen.emit(Generator::Prims);
        });
        this->append_item(U"Aldous Broder").connect([this] {
            chosen.emit(Generator::Aldous_broder);
        });
        this->append_item(U"Recursive Division").connect([this] {
            chosen.emit(Generator::Recursive_division);
        });

        *this | descendants() | fg(color::Almost_white) |
            bg(color::Almost_black);
    }
};

class Front_page
    : public ox::VTuple<Generator_title, ox::HLine, Generator_menu> {
   public:
    Generator_menu& menu = this->get<2>();
    ox::HLine& line      = this->get<1>();

   public:
    Front_page()
    {
        using namespace ox::pipe;
        *this | direct_focus() | forward_focus(menu);
        line | fg(color::Start) | bg(color::Almost_black);
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

        *this | direct_focus() | forward_focus(maze_stack);

        // middle stack
        this->get<1>().set_active_page(maze_stack_index_);

        maze_stack.maze_reset.connect([this] {
            bottom_bar.reset_notify.set_text(
                U"  Reset  " | bg(color::Reset_notify_bg) | fg(color::Black));
        });

        maze_stack.page_changed.connect(
            [this](auto const index) { top_bar.set_level_text(index + 1); });

        maze_stack.attempts_modified.connect(
            [this](auto count) { bottom_bar.attempts.set(count); });

        maze_stack.game_over.connect([this](auto const& attempts) {
            last_main_index_ = end_screen_index_;
            end_screen.initialize(attempts);
            top_bar.display_game_over_text();
            this->get<1>().set_active_page(end_screen_index_);
        });

        bottom_bar.reset_btn |
            on_press([this] { maze_stack.reset_active_maze(); });

        bottom_bar.about_btn.top | on_press([this] {
            this->get<1>().set_active_page(about_page_index_);
        });

        bottom_bar.about_btn.bottom | on_press([this] {
            this->get<1>().set_active_page(last_main_index_);
        });
    }

   public:
    void flip_to_mazes() { this->get<1>().set_active_page(maze_stack_index_); }

   private:
    std::size_t last_main_index_ = maze_stack_index_;

   private:
    static constexpr auto maze_stack_index_ = 0;
    static constexpr auto about_page_index_ = 1;
    static constexpr auto end_screen_index_ = 2;
};

class Lantern_app : public ox::SPair<Front_page, Full_UI> {
   public:
    Front_page& front_page = this->first;
    Full_UI& full_ui       = this->second;

   public:
    Lantern_app()
    {
        using namespace ox::pipe;
        *this | direct_focus() | active_page(front_page_index_);

        front_page.menu.chosen.connect([this](Generator g) {
            this->set_active_page(full_ui_index_);
            full_ui.maze_stack.initialize_mazes(g);
            full_ui.flip_to_mazes();
        });
    }

   private:
    static constexpr auto front_page_index_ = 0;
    static constexpr auto full_ui_index_    = 1;
};

}  // namespace lantern
#endif  // LANTERN_FULL_UI_HPP
