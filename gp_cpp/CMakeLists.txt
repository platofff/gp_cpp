cmake_minimum_required(VERSION 3.22)
project(genpattern)

option(BUILD_TESTS "Build the test executables" OFF)

if ("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clang:-Wall /clang:-Wextra /clang:-Wno-sign-compare")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /clang:-Wall /clang:-Wextra /clang:-Wno-sign-compare")

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL /clang:-O3 /clang:-ffp-contract=fast")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /GL /clang:-O3 /clang:-ffp-contract=fast")

    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-sign-compare")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-sign-compare")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -shared-libasan")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffp-contract=fast -fvisibility=hidden -fPIC")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -ffp-contract=fast -fvisibility=hidden -fPIC")

    set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
endif()

set(SOURCE_FILES
    "src/ImgAlpha.cpp"
    "src/ImgAlphaFilledContour.cpp"
    "src/misc.cpp"
    "src/OffsettedBitImage.cpp"
    "src/genpattern.cpp"
    "src/BitImage.cpp"
    "src/Canvas.cpp"
    "src/PatternGenerator.cpp"
)

set(HEADER_FILES
    "include/ImgAlpha.hpp"
    "include/ImgAlphaFilledContour.hpp"
    "include/misc.hpp"
    "include/OffsettedBitImage.hpp"
    "include/genpattern.h"
    "include/BitImage.hpp"
    "include/Canvas.hpp"
    "include/PatternGenerator.hpp"
    "include/CoolingSchedules.hpp"
)

if (EMSCRIPTEN)
    set(BUILD_TESTS OFF)
    add_executable(genpattern ${SOURCE_FILES} ${HEADER_FILES})
    set_target_properties(genpattern PROPERTIES LINK_FLAGS "-s SINGLE_FILE --bind --post-js ${CMAKE_CURRENT_SOURCE_DIR}/wasm/worker.js")
else()
    add_library(genpattern SHARED ${SOURCE_FILES} ${HEADER_FILES})
endif()

set_property(TARGET genpattern PROPERTY CXX_STANDARD 23)
target_include_directories(genpattern PRIVATE "include")

if (BUILD_TESTS)
    # Find GoogleTest.
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})

    # Create the standard test executable linked with the genpattern library.
    add_executable(genpattern_test
        ${SOURCE_FILES}
        "tests/test.cpp"
    )
    set_property(TARGET genpattern_test PROPERTY CXX_STANDARD 23)
    target_include_directories(genpattern_test PRIVATE "include")
    target_link_libraries(genpattern_test PRIVATE GTest::GTest GTest::Main)
    
    # Register the test with CTest.
    add_test(NAME genpattern_test COMMAND genpattern_test)

    # Create a test executable that uses the C API via dlopen() without static linking.
    add_executable(genpattern_test_lib "tests/test_lib.cpp")
    set_property(TARGET genpattern_test_lib PROPERTY CXX_STANDARD 23)
    target_include_directories(genpattern_test_lib PRIVATE "include")
    target_link_libraries(genpattern_test_lib PRIVATE ${CMAKE_DL_LIBS} GTest::GTest GTest::Main)
    
    # Register the second test.
    add_test(NAME genpattern_test_lib COMMAND genpattern_test_lib)
endif()
