cmake_minimum_required(VERSION 2.8.4)
project(ahBakeTransform)
if(NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wno-narrowing -march=native -m64 -O3 -static -funroll-loops")
endif()

FIND_PACKAGE( OpenMP REQUIRED)
if(OPENMP_FOUND)
	message("OPENMP FOUND")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(SOURCE_FILES
    src/ahBakeTransform.cpp src/tinyexr.h src/geometry.h src/stb_image_resize.h src/stb_image_write.h )

add_executable(ahBakeTransform ${SOURCE_FILES})