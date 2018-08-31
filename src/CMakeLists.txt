SET(GCC_COVERAGE_COMPILE_FLAGS "-Wno-sizeof-array-argument")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})
add_executable (ps3-iso-validate ps3-iso-validate.c zefie.c md5.c manamain.c zpipe.c iso.c ird.c)
TARGET_LINK_LIBRARIES(ps3-iso-validate z)
