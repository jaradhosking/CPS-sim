cmake_minimum_required(VERSION 3.15)
project(SampleSimulation)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(CPS-sim
        model.c
        engine.c
        README.md
        sim.h)
