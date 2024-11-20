# SPDX-License-Identifier: LGPL-3.0-or-later
set(SRC_FILES main.cpp)
set(HDR_FILES)
include_directories(${PROJECT_SOURCE_DIR}/src)

add_executable(clserpp-repl ${SRC_FILES} ${HDR_FILES})

target_link_libraries(clserpp-repl clserpp)
