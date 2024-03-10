if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL /clang:-Ofast /clang:-flto")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /GL /clang:-Ofast /clang:-flto")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /clang:-flto")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -flto")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Ofast -flto")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
endif()

set(SOURCE_FILES
    "src/ImgAlpha.cpp"
    "src/misc.cpp"
    "src/OffsettedImgAlpha.cpp"
    "src/genpattern.cpp"
)

add_library(genpattern SHARED
    ${SOURCE_FILES}
    "include/ImgAlpha.hpp"
    "include/BitImage.hpp"
    "include/misc.hpp"
    "include/OffsettedImgAlpha.hpp"
    "include/AnnealingSimulator.hpp"
    "include/genpattern.h"
)

option(BUILD_TESTS "Build the test executable" ON)
if (BUILD_TESTS)
    add_executable(genpattern_test
        ${SOURCE_FILES}
        "tests/test.cpp"
    )
    target_include_directories(genpattern_test PRIVATE "include")
endif()

set_property(TARGET genpattern PROPERTY CXX_STANDARD 23)
target_include_directories(genpattern PRIVATE "include")
