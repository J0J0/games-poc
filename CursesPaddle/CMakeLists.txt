cmake_minimum_required(VERSION 2.6)

project(CursesPaddle)
find_package(Curses)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")

add_executable(CursesPaddle cursespaddle.c)
target_link_libraries(CursesPaddle ${CURSES_LIBRARIES} rt)
