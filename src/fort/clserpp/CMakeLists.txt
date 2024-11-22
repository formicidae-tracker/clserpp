# SPDX-License-Identifier: LGPGL-3.0-or-later

set(HDR_FILES clser.h clserpp.hpp details.hpp)
set(TEST_SRC_FILES buffer.cpp read_buffer.cpp)
set(TEST_HDR_FILES)

add_library(clserpp SHARED ${SRC_FILES} ${HDR_FILES})

target_link_libraries(clserpp PUBLIC cpptrace::cpptrace spdlog::spdlog_header_only)
target_link_libraries(clserpp INTERFACE ${CLSER_LIBRARY})

if(CLSERPP_IMPORTED)
	add_library(fort-clserpp::clserpp INTERFACE IMPORTED GLOBAL)
	target_link_libraries(
		fort-clserpp::clserpp INTERFACE clserpp cpptrace::cpptrace
	)
	target_include_directories(
		fort-clserpp::clserpp INTERFACE ${PROJECT_SOURCE_DIR}/src
	)
else()
	add_executable(clserpp-tests ${TEST_SRC_FILES} ${TEST_HDR_FILES})
	target_link_libraries(clserpp-tests clserpp GTest::gtest_main)
	gtest_discover_tests(clserpp-tests)
	add_dependencies(check clserpp-tests)
endif()

install(FILES ${HDR_FILES} DESTINATION include/fort/clserpp)
