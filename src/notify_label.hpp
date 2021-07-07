#ifndef LANTERN_NOTIFY_LABEL_HPP
#define LANTERN_NOTIFY_LABEL_HPP
#include <chrono>
#include <utility>

#include <termox/painter/glyph_string.hpp>
#include <termox/widget/widgets/label.hpp>

namespace lantern {

/// Temporarily displays a label, then hides it.
class Notify_label : public ox::HLabel {
   public:
    using ox::HLabel::HLabel;

   public:
    /// Set the text to display and start counting towards the interval.
    void set_text(ox::Glyph_string label)
    {
        this->ox::HLabel::set_text(std::move(label));
        this->enable_animation(interval_);
    }

    /// Set the interval to display the label when set.
    void set_interval(std::chrono::milliseconds x);

   protected:
    auto timer_event() -> bool override
    {
        this->ox::HLabel::set_text(U"");
        this->disable_animation();
        return ox::HLabel::timer_event();
    }

   private:
    std::chrono::milliseconds interval_ = std::chrono::milliseconds{2'000};
};

}  // namespace lantern
#endif  // LANTERN_NOTIFY_LABEL_HPP
