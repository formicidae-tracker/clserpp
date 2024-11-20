# SPDX-License-Identifier: LGPGL-3.0-or-later

cmake_minimum_required(VERSION 3.10)

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
