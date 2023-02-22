cmake_minimum_required(VERSION 3.1)

project(CursesPaddle LANGUAGES C)
find_package(Curses)
add_executable(CursesPaddle cursespaddle.c)
target_link_libraries(CursesPaddle ${CURSES_LIBRARIES} rt)
