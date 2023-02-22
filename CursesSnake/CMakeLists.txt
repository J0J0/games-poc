cmake_minimum_required(VERSION 3.1)

project(CursesSnake LANGUAGES C)
find_package(Curses)
add_executable(CursesSnake cursessnake.c)
target_link_libraries(CursesSnake ${CURSES_LIBRARIES} rt)
