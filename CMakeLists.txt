cmake_minimum_required(VERSION 3.25)
project(Adamastor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

set(SRC
    src/main.cc
    src/parse-options.cc
    src/replay-util.cc
)

add_executable(adamastor ${SRC})

find_package(boost_beast CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(GTest CONFIG REQUIRED)

target_link_libraries(adamastor PRIVATE Boost::beast OpenSSL::SSL OpenSSL::Crypto)

target_include_directories(adamastor PRIVATE src)

target_compile_options(adamastor PRIVATE
    -Wall -Werror -Wextra
    -Wformat-security
    -Wpointer-arith
    -Woverflow
    -Wvla
    -Wunused
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wno-unused-parameter
    -Wno-missing-field-initializers
)

set(CLANG_FORMAT clang-format CACHE STRING "clang-format binary")
set(CLANG_TIDY clang-tidy CACHE STRING "clang-tidy binary")

file(GLOB FMT_FILES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cc
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/tests/*.cc
)

add_custom_target(fmt
    COMMAND ${CLANG_FORMAT} -i ${FMT_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(fmt-check
    COMMAND ${CLANG_FORMAT} --dry-run -Werror ${FMT_FILES}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

add_custom_target(tidy
    COMMAND ${CLANG_TIDY} -p ${CMAKE_BINARY_DIR} ${SRC}
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

enable_testing()
include(GoogleTest)

file(GLOB TEST_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/*.cc)

foreach(test_file ${TEST_FILES})
    get_filename_component(test_name ${test_file} NAME_WE)
    add_executable(${test_name} ${test_file})
    target_include_directories(${test_name} PRIVATE src)
    target_link_libraries(${test_name} PRIVATE GTest::gtest GTest::gtest_main)
    gtest_discover_tests(${test_name})
endforeach()

