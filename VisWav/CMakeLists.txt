cmake_minimum_required(VERSION 4.0)
project(VisWav)

find_package(SDL3 REQUIRED)
find_package(FFTW3 REQUIRED)


set(WAV_SOURCES ./src/read_wav.c ./src/fft.c)


add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PUBLIC ${WAV_SOURCES} ./src/plot.c)
target_include_directories(${PROJECT_NAME} PUBLIC ./include)
target_link_libraries(${PROJECT_NAME} PUBLIC SDL3::SDL3 fftw3 m)
