cmake_minimum_required(VERSION 3.1)

project(CursesBomber LANGUAGES C)
find_package(Curses)
add_executable(CursesBomber cursesbomber.c)
target_link_libraries(CursesBomber ${CURSES_LIBRARIES} rt)
