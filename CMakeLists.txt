cmake_minimum_required(VERSION 3.16)
project(lk C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(LK_INSTALL_PATH "/usr/local" CACHE PATH "Install prefix for lk")
option(LK_ASAN "Enable AddressSanitizer" OFF)

add_compile_options(
    -Wall -Wextra -pedantic
    "$<$<CONFIG:Debug>:-O0;-g>"
    "$<$<CONFIG:Release>:-O2>"
)

if(LK_ASAN)
    add_compile_options(-fsanitize=address -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address)
endif()

# Core library (object library avoids double-compiling for future shared lib use)
file(GLOB RT_SOURCES CONFIGURE_DEPENDS "src/rt/*.c")
file(GLOB CORE_SOURCES CONFIGURE_DEPENDS "src/*.c")
list(REMOVE_ITEM CORE_SOURCES "${CMAKE_SOURCE_DIR}/src/main.c")

add_library(lkcore OBJECT ${CORE_SOURCES} ${RT_SOURCES})
target_include_directories(lkcore PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/rt
)

# Main executable
add_executable(lk src/main.c $<TARGET_OBJECTS:lkcore>)
target_include_directories(lk PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/rt
)
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
