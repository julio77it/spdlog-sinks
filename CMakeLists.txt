cmake_minimum_required(VERSION 3.13.4)
project(Spdlog-Sinks VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

#
# CONAN DEPENDENCIES
#
if (NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
    message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
    file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.12/conan.cmake"
            	  "${CMAKE_BINARY_DIR}/conan.cmake")
endif ()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(BASIC_SETUP
		CONANFILE conanfile.txt
		CMAKE_TARGETS
		BUILD missing)


#
# Project Configuration
#
set(CMAKE_BUILD_TYPE Release)
set(WARNING_FLAGS "-fno-omit-frame-pointer -fconcepts -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wold-style-cast -Wcast-align -Wunused -Woverloaded-virtual -Wpedantic -Wconversion -Wsign-conversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wnull-dereference -Wuseless-cast -Wdouble-promotion -Wformat=2")

set(CMAKE_C_FLAGS_DEBUG "-g ${WARNING_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "-O4 ${WARNING_FLAGS}")

include_directories(${CMAKE_SOURCE_DIR}/include
		    ${CMAKE_BINARY_DIR}/include)


#
# example
#
add_executable(example
			   examples/example.cpp)
			   
target_link_libraries(example
		      		  PRIVATE CONAN_PKG::spdlog)

target_include_directories(example
			   			   PRIVATE CONAN_PKG::spdlog)

