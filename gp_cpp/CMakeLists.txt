option(BUILD_TESTS "Build the test executable" ON)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL /clang:-Ofast /clang:-flto")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /GL /clang:-Ofast /clang:-flto")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /clang:-flto")
else()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -stdlib=libc++")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -stdlib=libc++")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
endif()

set(SOURCE_FILES
    "src/ImgAlpha.cpp"
    "src/misc.cpp"
    "src/OffsettedImgAlpha.cpp"
    "src/genpattern.cpp"
    "src/AnnealingSimulation.cpp"
    "src/BitImage.cpp"
    "src/Canvas.cpp"
    "src/PatternGenerator.cpp"
)

set(HEADER_FILES
    "include/ImgAlpha.hpp"
    "include/BitImage.hpp"
    "include/misc.hpp"
    "include/OffsettedImgAlpha.hpp"
    "include/AnnealingSimulation.hpp"
    "include/genpattern.h"
)

if (EMSCRIPTEN)
    set(BUILD_TESTS OFF)
    add_executable(genpattern ${SOURCE_FILES} ${HEADER_FILES})
    set_target_properties(genpattern PROPERTIES LINK_FLAGS "--bind --post-js ${CMAKE_CURRENT_SOURCE_DIR}/wasm/worker.js")
else()
    add_library(genpattern SHARED ${SOURCE_FILES} ${HEADER_FILES})
endif()

set_property(TARGET genpattern PROPERTY CXX_STANDARD 23)

if (BUILD_TESTS)
    add_executable(genpattern_test
        ${SOURCE_FILES}
        "tests/test.cpp"
    )
    set_property(TARGET genpattern_test PROPERTY CXX_STANDARD 23)
    target_include_directories(genpattern_test PRIVATE "include")
endif()

target_include_directories(genpattern PRIVATE "include")

