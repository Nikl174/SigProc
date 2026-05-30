cmake_minimum_required(VERSION 4.0)
project(VisWav)

find_package(SDL3 REQUIRED)
find_package(FFTW3 REQUIRED)

set(WAV_SOURCES 
  ./src/read_wav.c
  ./src/fft.c
)

include_directories(./include/)

add_executable(PlotWav)
target_sources(PlotWav PUBLIC ${WAV_SOURCES} ./src/plot.c)
target_link_libraries(PlotWav PUBLIC SDL3::SDL3 fftw3 m)
