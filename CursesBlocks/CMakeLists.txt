cmake_minimum_required(VERSION 3.1)

project(CursesBlocks LANGUAGES C)
find_package(Curses)
add_executable(CursesBlocks cursesblocks.c)
target_link_libraries(CursesBlocks ${CURSES_LIBRARIES} rt)
