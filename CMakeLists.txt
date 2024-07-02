cmake_minimum_required(VERSION 3.28)
project(ai1027_hausarbeit_03 C)

set(CMAKE_C_STANDARD 23)

add_executable(ai1027_hausarbeit_03 src/main.c
        src/publisher.c
        src/subscriber.c
        src/broker.c
        src/broker.h
        src/publisher.h
        src/subscriber.h
        src/main.h)
