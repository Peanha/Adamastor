cmake_minimum_required(VERSION 3.25)
project(Adamastor VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_definitions(DEBUG)
endif()

set(SRC
    src/main.cc
    src/parse-options.cc
    src/replay-util.cc
)

add_executable(adamastor ${SRC})

find_package(boost_beast CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

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
