cmake_minimum_required(VERSION 3.16)
project(ani C)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

option(ANI_JSON "JSON backend" "yyjson")
option(ANI_WITH_UTF8PROC "Enable utf8proc normalization" ON)
option(ANI_SANITIZE "Enable ASAN/UBSAN in Debug" ON)

if (ANI_SANITIZE AND CMAKE_BUILD_TYPE MATCHES "Debug")
  add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g3 -O1)
  add_link_options(-fsanitize=address,undefined)
endif()

add_subdirectory(third_party)
add_subdirectory(src)
enable_testing()
add_subdirectory(tests)
