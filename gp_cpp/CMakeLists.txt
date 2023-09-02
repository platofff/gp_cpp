add_library (genpattern SHARED 
  "include/ImgAlpha.hpp"
  "src/ImgAlpha.cpp"
  "include/BitImage.hpp"
  "include/misc.hpp"
  "src/misc.cpp"
  "include/OffsettedImgAlpha.hpp"
  "src/OffsettedImgAlpha.cpp"
  "include/AnnealingSimulator.hpp"
  "include/genpattern.h"
  "src/genpattern.cpp" )

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(TARGET genpattern PROPERTY CXX_STANDARD 23)

find_package(mdspan CONFIG REQUIRED)
target_link_libraries(genpattern PRIVATE std::mdspan)

target_include_directories(genpattern PRIVATE ${mdspan_INCLUDE_DIRS} "include")
