#ifndef LANTERN_FLICKER_HPP
#define LANTERN_FLICKER_HPP
#include <termox/painter/dynamic_colors.hpp>

namespace lantern {

class Flicker : private ox::dynamic::Modulation_base {
   public:
    /// Resolution is the number of steps to complete a full cycle.
    /** Offset is the step to start at. */
    Flicker(unsigned resolution, unsigned offset);

   public:
    /// Returns value in range [0.0, 1.0]
    [[nodiscard]] auto operator()() -> double;
};

// TODO inherit from flicker to make different offset starts.

}  // namespace lantern
#endif  // LANTERN_FLICKER_HPP
