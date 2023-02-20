cmake_minimum_required(VERSION 2.6)

project(CursesBomber)
find_package(Curses)
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -std=gnu99")

add_executable(CursesBomber cursesbomber.c)
target_link_libraries(CursesBomber ${CURSES_LIBRARIES} rt)
