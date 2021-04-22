cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
cmake_policy(VERSION 3.0)

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON CACHE INTERNAL "")

set(CMAKE_C_COMPILER "emcc")
set(CMAKE_CXX_COMPILER "emcc")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

## set cmake C flags for emcc
set(
  CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -Os -s USE_GLFW=3 -s ASSERTIONS=1 -s WASM=1 -s ASYNCIFY"
)

# Since WASM is used, ALLOW_MEMORY_GROWTH has no extra overheads
set(
  CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} -s ALLOW_MEMORY_GROWTH=1 --no-heap-copy"
)

set(
  CMAKE_C_FLAGS
  "${CMAKE_C_FLAGS} --shell-file ${CMAKE_SOURCE_DIR}/src/shell.html"
)

set(CMAKE_EXE_LINKER_FLAGS "-s USE_GLFW=3")

# set executable to HTML for webasm
set(CMAKE_EXECUTABLE_SUFFIX ".html")

# Remove the -rdynamic flag because otherwise emscripten does not generate
# HTML+JS+WASM files, only a non-working and fat HTML
string(
  REPLACE
    "-rdynamic" ""
    CMAKE_SHARED_LIBRARY_LINK_C_FLAGS
    "${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS}"
)

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

# adds dependencies in correct order
add_subdirectory(third-party)
#add_subdirectory(libraries)
add_subdirectory(application)
