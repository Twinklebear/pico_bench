add_executable(simple simple.cpp)
install(TARGETS simple DESTINATION ${PROJECT_BINARY_DIR})

add_executable(mandelbrot mandelbrot.cpp)
install(TARGETS mandelbrot DESTINATION ${PROJECT_BINARY_DIR})

