cmake_minimum_required (VERSION 2.8)
set(CMAKE_LEGACY_CYGWIN_WIN32 0)

project (ps3-iso-validate)

set(SOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
SET(STATIC_BUILD 1)
SET(GCC_COVERAGE_COMPILE_FLAGS "-D_FILE_OFFSET_BITS=64 -O3 -static")
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

add_executable (ps3-iso-validate ${SOURCES_DIR}/ps3-iso-validate.c ${SOURCES_DIR}/zefie.c ${SOURCES_DIR}/md5.c ${SOURCES_DIR}/manamain.c ${SOURCES_DIR}/zpipe.c ${SOURCES_DIR}/iso.c ${SOURCES_DIR}/ird.c)

TARGET_LINK_LIBRARIES(ps3-iso-validate z ws2_32)
