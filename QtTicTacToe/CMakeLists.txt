cmake_minimum_required(VERSION 3.1)

project(QtTicTacToe LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

# Support both, Qt5 and Qt6. Taken from
#   https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html#supporting-older-qt-5-versions

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

if(QT_VERSION_MAJOR EQUAL 6)
    set(CMAKE_CXX_STANDARD 17)
endif()

add_executable(QtTicTacToe
    gameboard.cpp gameboard.hpp
    gamefield.cpp gamefield.hpp
    main.cpp
)
target_link_libraries(QtTicTacToe Qt${QT_VERSION_MAJOR}::Widgets)
