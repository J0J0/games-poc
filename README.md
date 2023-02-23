# Proof-of-concept implementations of some (video) games

I wrote most of them (years ago) for students who just got introduced to C,
in order to demonstrate that it is not too ambitious to write a simple game
as a final project. Others were just fun projects of mine that fit into this
repository.


## [CliMaedn](CliMaedn/)
Pure terminal version of the German board game classic ["Mensch ärgere Dich nicht"](https://en.wikipedia.org/wiki/Mensch_ärgere_Dich_nicht).


## [CursesBlocks](CursesBlocks/)
Ncurses version of a well known puzzle game that features falling [tetrominoes](https://en.wikipedia.org/wiki/Tetromino).

Best played on a terminal font with square glyphs, e.g. [Square](https://strlen.com/square/).


## [CursesBomber](CursesBomber/)
Ncurses game, inspired by Bomberman.


## [CursesPaddle](CursesPaddle/)
Ncurses version of another video game classic, featuring two paddles and a "ball".


## [CursesSnake](CursesSnake/)
Ncurses game, inspired by snake.

Best played on a terminal font with square glyphs, e.g. [Square](https://strlen.com/square/).


## [NoGardenPuzzle](NoGardenPuzzle/)
Rudimentary reimplementation in Haskell (using [gloss](https://hackage.haskell.org/package/gloss))
of the [Zen Garden game on Steam](https://store.steampowered.com/app/592660/Zen_Garden/).


## [QtTicTacToe](QtTicTacToe/)
Minimal implementation of [tic-tac-toe](https://en.wikipedia.org/wiki/Tic-tac-toe) in C++/Qt.


# Build instructions
Use `make` (or e.g. `CMAKE_GENERATOR=Ninja make`) to build everything. The executables will be in `BUILD/bin`.
For prerequisites see the readme files of individual project directories.

# License
Unless stated otherwise in a subdirectory, everything in this repository is released
under the [ISC License](LICENSE).
