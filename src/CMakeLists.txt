add_library(pico_bench pico_bench.cpp)
set_target_properties(pico_bench PROPERTIES PUBLIC_HEADER ../include/pico_bench.h)
install(TARGETS pico_bench
	ARCHIVE DESTINATION lib
	PUBLIC_HEADER DESTINATION include)

