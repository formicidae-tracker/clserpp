# SPDX-License-Identifier: LGPGL-3.0-or-later

set(HDR_FILES clser.h clserpp.hpp details.hpp)
set(SRC_FILES clserpp.cpp)
set(TEST_SRC_FILES test.cpp)
set(TEST_HDR_FILES)
add_library(clserpp ${SRC_FILES} ${HDR_FILES})
target_link_libraries(clserpp ${CLSER_LIBRARY})

if(CLSERPP_IMPORTED)
	add_library(fort-clserpp::clserpp INTERFACE IMPORTED GLOBAL)
	target_link_libraries(
		fort-clserpp::clserpp INTERFACE clserpp ${CLSER_LIBRARY}
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
