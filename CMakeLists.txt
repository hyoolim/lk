cmake_minimum_required(VERSION 3.16)
project(lk C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(LK_INSTALL_PATH "/usr/local" CACHE PATH "Install prefix for lk")

add_compile_options(-O2 -g -Wall -pedantic)

# Core library (object library avoids double-compiling for future shared lib use)
file(GLOB BASE_SOURCES "src/base/*.c")
file(GLOB CORE_SOURCES "src/*.c")
list(REMOVE_ITEM CORE_SOURCES "${CMAKE_SOURCE_DIR}/src/main.c")

add_library(lkcore OBJECT ${CORE_SOURCES} ${BASE_SOURCES})
target_include_directories(lkcore PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/base
)

# Main executable
add_executable(lk src/main.c $<TARGET_OBJECTS:lkcore>)
target_include_directories(lk PRIVATE ${CMAKE_SOURCE_DIR}/include)
target_compile_definitions(lk PRIVATE LK_INSTALL_PATH="${LK_INSTALL_PATH}")

if(CMAKE_SYSTEM_NAME MATCHES "Darwin|Linux")
    target_link_libraries(lk dl)
endif()
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(lk m)
endif()

# Build .lk stdlib libraries (equivalent to `make lib`)
add_custom_target(stdlib
    COMMAND lk -l ${CMAKE_SOURCE_DIR}/stdlib ${CMAKE_SOURCE_DIR}/buildlibs.lk
    DEPENDS lk
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Building lk stdlib libraries"
)

# Install
install(TARGETS lk DESTINATION bin)
install(DIRECTORY ${CMAKE_SOURCE_DIR}/stdlib/ DESTINATION lib/lk)
install(FILES ${CMAKE_SOURCE_DIR}/include/*.h DESTINATION include/lk)
install(FILES ${CMAKE_SOURCE_DIR}/include/base/*.h DESTINATION include/lk/base)

# Tests
enable_testing()
file(GLOB_RECURSE LK_TESTS "${CMAKE_SOURCE_DIR}/tests/*.lk")
foreach(test_file ${LK_TESTS})
    get_filename_component(test_name ${test_file} NAME_WE)
    get_filename_component(test_dir ${test_file} DIRECTORY)
    add_test(
        NAME ${test_name}
        COMMAND lk -l ${CMAKE_SOURCE_DIR}/stdlib ${test_file}
        WORKING_DIRECTORY ${test_dir}
    )
endforeach()
