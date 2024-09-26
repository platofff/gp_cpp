option(BUILD_TESTS "Build the test executable" ON)

if ("${CMAKE_CXX_COMPILER_FRONTEND_VARIANT}" MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /clang:-Wall /clang:-Wextra /clang:-Wno-sign-compare")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGSS} /clang:-Wall /clang:-Wextra /clang:-Wno-sign-compare")

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /GL /clang:-O3 /clang:-ffp-contract=fast /clang:-flto")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /O2 /GL /clang:-O3 /clang:-ffp-contract=fast /clang:-flto")
    
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /clang:-flto")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra /clang:-Wno-sign-compare")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGSS} -Wall -Wextra /clang:-Wno-sign-compare")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -stdlib=libc++ -fsanitize=address -shared-libasan")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffp-contract=fast -stdlib=libc++")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3 -ffp-contract=fast")

    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -flto")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")
endif()

set(SOURCE_FILES
    "src/ImgAlpha.cpp"
    "src/ImgAlphaFilledContour.cpp"
    "src/misc.cpp"
    "src/OffsettedImgAlpha.cpp"
    "src/genpattern.cpp"
    "src/BitImage.cpp"
    "src/Canvas.cpp"
    "src/PatternGenerator.cpp"
    "src/CoolingSchedules.cpp"
)

set(HEADER_FILES
    "include/ImgAlpha.hpp"
    "include/ImgAlphaFilledContour.hpp"
    "include/misc.hpp"
    "include/OffsettedImgAlpha.hpp"
    "include/genpattern.h"
    "include/BitImage.hpp"
    "include/Canvas.hpp"
    "include/PatternGenerator.hpp"
    "include/CoolingSchedules.hpp"
)

if (EMSCRIPTEN)
    set(BUILD_TESTS OFF)
    add_executable(genpattern ${SOURCE_FILES} ${HEADER_FILES})
    set_target_properties(genpattern PROPERTIES LINK_FLAGS "--bind --post-js ${CMAKE_CURRENT_SOURCE_DIR}/wasm/worker.js")
else()
    add_library(genpattern SHARED ${SOURCE_FILES} ${HEADER_FILES})
endif()

set_property(TARGET genpattern PROPERTY CXX_STANDARD 23)
target_include_directories(genpattern PRIVATE "include")

if (BUILD_TESTS)
    add_executable(genpattern_test
        ${SOURCE_FILES}
        "tests/test.cpp"
    )
    set_property(TARGET genpattern_test PROPERTY CXX_STANDARD 23)
    target_include_directories(genpattern_test PRIVATE "include")
endif()

