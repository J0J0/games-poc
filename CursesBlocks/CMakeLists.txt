cmake_minimum_required(VERSION 2.6)

project(CursesBlocks)
find_package(Curses)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")

add_executable(CursesBlocks cursesblocks.c)
target_link_libraries(CursesBlocks ${CURSES_LIBRARIES} rt)
