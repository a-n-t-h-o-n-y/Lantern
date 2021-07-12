#ifndef LANTERN_GENERATOR_HPP
#define LANTERN_GENERATOR_HPP
namespace lantern {

/// Types of maze generators.
enum class Generator {
    Recursive_backtracking,
    Kruskal,
    Prims,
    Aldous_broder,
    Recursive_division,
};

}  // namespace lantern
#endif  // LANTERN_GENERATOR_HPP
