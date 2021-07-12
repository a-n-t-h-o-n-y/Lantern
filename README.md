# Lantern 🕯️

Complete each maze with minimal visiblity, making no wrong turns.

The Wanderer is sent back to the beginning whenever the minimum number of steps
to complete the maze has been reached(without making it to the end).

Each attempt is tallied.

<p align="center">
  <img src="docs/lantern-1.png">
</p>

## Controls

- Movement: Arrow Keys / `wasd` / `hjkl`
- Reset Level: `r`
- Exit App: `Ctrl c`

<p align="center">
  <img src="docs/lantern-2.png">
</p>

## Build

git submodules are used to pull in dependencies, run `git submodule update
--init --recursive` after cloning the repo.

CMake is the supported build generator, it generates the `lantern` executable
target. Requires a C++20(concepts) capable compiler.

## Cheats

You didn't hear it from me, but if you press `Shift` + `N`, you will jump
forward one level.
