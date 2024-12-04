cmake_minimum_required(VERSION 3.29)
project(playclist C)

set(CMAKE_C_STANDARD 17)

include_directories(include)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(playclist main.c
        include/type.h
        include/ncutil.h
        src/song.c
        include/song.h
        src/ncutil.c)

target_link_libraries(playclist ${CURSES_LIBRARIES})
