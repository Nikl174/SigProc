cmake_minimum_required(VERSION 4.0)
project(VisWav)

find_package(SDL3 REQUIRED)

set(WAV_SOURCES ./src/read_wav.c)

include_directories(./include/)

add_executable(PlotWav)
target_sources(PlotWav PUBLIC ${WAV_SOURCES} ./src/plot.c)
target_link_libraries(PlotWav PUBLIC SDL3::SDL3)
