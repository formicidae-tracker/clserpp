# SPDX-License-Identifier: LGPGL-3.0-or-later

cmake_minimum_required(VERSION 3.16)

project(
	clserpp
	LANGUAGES C CXX
	VERSION 0.0.1
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED 1)

if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
	set(CLSERPP_IMPORTED 1)
endif()

option(CLSER_LIBRARY_NAME "Name of the manufacturer provided clser library"
	   clsermv
)
option(CLSER_LIBRARY_NAMES
	   "Aditional names for the manufacturer provided clser library" ""
)
option(CLSER_LIBRARY_PATHS
	   "Additional path to find the manufacturer provided clser library" ""
)

option(CLSERPP_BUILD_TOOLS "Build tools for clserpp" On)

find_library(
	CLSER_LIBRARY ${CLSER_LIBRARY_NAME}
	NAMES ${CLSER_LIBRARY_NAMES}
	PATHS ${CLSER_LIBRARY_PATHS}
	PATH_SUFFIXES lib lib/${CMAKE_HOST_SYSTEM_PROCESSOR}
)

if(CLSER_LIBRARY)
	message(STATUS "clser library: ${CLSER_LIBRARY}")
else()
	message(FATAL_ERROR "could not found clser library")
endif()

include(FetchContent)
FetchContent_Declare(
	cpptrace
	GIT_REPOSITORY https://github.com/jeremy-rifkin/cpptrace.git
	GIT_TAG v0.7.3
)
FetchContent_MakeAvailable(cpptrace)

if(NOT CLSERPP_IMPORTED)

	FetchContent_Declare(
		googletest
		GIT_REPOSITORY https://github.com/google/googletest.git
		GIT_TAG 9bb354fa8325fa31faa1e12627b25ab445e6eed3
	)
	# For Windows: Prevent overriding the parent project's compiler/linker
	# settings
	set(gtest_force_shared_crt
		ON
		CACHE BOOL "
	" FORCE
	)

	FetchContent_MakeAvailable(googletest)
	enable_testing()

	include(GoogleTest)

	add_custom_target(check ALL ${CMAKE_CTEST_COMMAND} ARGS --output-on-failure)
endif()

add_subdirectory(src/fort/clserpp)

if(CLSERPP_BUILD_TOOLS)
	add_subdirectory(src/fort/clserpp-repl)
endif()
