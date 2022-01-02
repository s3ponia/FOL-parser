cmake_minimum_required(VERSION 3.0)
project(fol_parser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_XCODE_GENERATE_SCHEME ON)
set(CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER ON)
set(CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
set(THREADS_PREFER_PTHREAD_FLAG ON)

include_directories(FOL-parser)

# Unit testing
file(GLOB TEST_SOURCES "FOL-parser/tests/*.cpp")
add_executable(tests ${TEST_SOURCES} FOL-parser/tests/main.cpp ${SOURCES})
target_link_libraries(tests ${CONAN_LIBS})
set_target_properties(tests
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/bin")

