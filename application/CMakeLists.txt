add_executable(ludum-dare-client)

target_include_directories(ludum-dare-client PUBLIC "include/")

target_sources(
  ludum-dare-client
  PRIVATE
    src/source.cpp
    src/scene.cpp
)

set_target_properties(
  ludum-dare-client
  PROPERTIES
    COMPILE_FLAGS
      "-Wshadow -Wdouble-promotion -Wall -Wformat=2 -Wextra -Wpedantic -Wundef"
)

target_link_libraries(
  ludum-dare-client
  PRIVATE
    raylib glfw
)

install(
  TARGETS ludum-dare-client
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT core
)

include(GNUInstallDirs)

# install additional wasm/js files
install(
  FILES
    ${CMAKE_BINARY_DIR}/application/ludum-dare-client.js
    ${CMAKE_BINARY_DIR}/application/ludum-dare-client.wasm
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT core
)
