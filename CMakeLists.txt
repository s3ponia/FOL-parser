cmake_minimum_required(VERSION 3.0)
project(fol_parser)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_XCODE_GENERATE_SCHEME ON)
set(CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER ON)
set(CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN ON)
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -fsanitize=address -O0")
set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS} -O2")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()
set(THREADS_PREFER_PTHREAD_FLAG ON)

include_directories(FirstOrderLogic)

file(GLOB SOURCES "FirstOrderLogic/libfol-prover/src/*.cpp" 
	"FirstOrderLogic/libfol-basictypes/src/*.cpp" 
	"FirstOrderLogic/libfol-matcher/src/*.cpp" 
	"FirstOrderLogic/libfol-parser/parser/src/*.cpp" 
	"FirstOrderLogic/libfol-parser/lexer/src/*.cpp"
	"FirstOrderLogic/libfol-transform/src/*.cpp"
	"FirstOrderLogic/libfol-unification/src/*.cpp"
	)

add_executable(fol_prover "FirstOrderLogic/main.cpp" ${SOURCES})

# Unit testing
file(GLOB TEST_SOURCES "FirstOrderLogic/tests/*.cpp")
add_executable(tests ${TEST_SOURCES}  ${SOURCES})
target_link_libraries(tests ${CONAN_LIBS})
set_target_properties(tests
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/tests/bin")

