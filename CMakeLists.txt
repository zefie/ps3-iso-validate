cmake_minimum_required (VERSION 2.8)
project (ps3-iso-validate)

set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

SET(GCC_COVERAGE_COMPILE_FLAGS "-Wno-sizeof-array-argument")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

add_executable (ps3-iso-validate ${SOURCES_DIR}/ps3-iso-validate.c ${SOURCES_DIR}/zefie.c ${SOURCES_DIR}/md5.c ${SOURCES_DIR}/manamain.c ${SOURCES_DIR}/zpipe.c ${SOURCES_DIR}/iso.c ${SOURCES_DIR}/ird.c)

TARGET_LINK_LIBRARIES(ps3-iso-validate z)
