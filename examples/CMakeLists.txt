add_executable(simple simple.cpp)
target_link_libraries(simple pico_bench)
install(TARGETS simple DESTINATION ${PROJECT_BINARY_DIR})

add_executable(mandelbrot mandelbrot.cpp)
target_link_libraries(mandelbrot pico_bench)
install(TARGETS mandelbrot DESTINATION ${PROJECT_BINARY_DIR})

