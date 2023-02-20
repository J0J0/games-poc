cmake_minimum_required(VERSION 2.6)

project(CursesSnake)
find_package(Curses)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_executable(CursesSnake cursessnake.c)
target_link_libraries(CursesSnake ${CURSES_LIBRARIES} rt)
