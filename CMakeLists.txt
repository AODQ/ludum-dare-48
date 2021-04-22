cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
cmake_policy(VERSION 3.0)

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON CACHE INTERNAL "")

set(CMAKE_C_COMPILER "emcc")
set(CMAKE_CXX_COMPILER "emcc")

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY")

SET(PLATFORM Web CACHE STRING "Forcing PLATFORM_WEB because EMSCRIPTEN was detected")

set(USE_EXTERNAL_GLFW ON CACHE INTERNAL "")
set(PLATFORM "Web" CACHE INTERNAL "")
set(EMSCRIPTEN ON CACHE INTERNAL "")

# add ccache if found
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
  set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK    ccache)
  message("*-- CCACHE found, will use")
endif()

project (
  ludum-dare
  VERSION 0.0
  LANGUAGES C CXX
  DESCRIPTION "2021 ludum dare"
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## requires out of source builds
file(TO_CMAKE_PATH "${PROJECT_BINARY_DIR}/CMakeLists.txt" LOC_PATH)
if (EXISTS "${LOC_PATH}")
  message(FATAL_ERROR "build must be out of source")
endif()

# adds dependencies in correct order
add_subdirectory(third-party)
#add_subdirectory(libraries)
add_subdirectory(application)
