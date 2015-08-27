add_executable(simple simple.cpp)
install(TARGETS simple DESTINATION ${PICO_BENCH_INSTALL_DIR})

add_executable(mandelbrot mandelbrot.cpp)
install(TARGETS mandelbrot DESTINATION ${PICO_BENCH_INSTALL_DIR})

