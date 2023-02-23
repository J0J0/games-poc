# NoGardenPuzzle
## Build instructions
Using the Haskell build system [`cabal`](https://www.haskell.org/cabal/):

    cabal build

The program can be run with `cabal run`.

Note, that the cmake file is basically just a wrapper around `cabal`.
For building only this project, using `cmake` has no additional benefits
over `cabal` directly.

## Game rules
The goal is to fill the rectangular game field with horizontal and vertical lines,
such that each line starts and ends "outside" of the field. A started line extends
in a straight manner until it hits a blocked tile or the outside. In the former
case, the line must be continued in either possible direction.

## Controls
Left mouse click on the arrows starts and continues lines. Right mouse click aborts
an unfinished line. The `R` key resets the board.
