add_executable(ludum-dare-client)

target_include_directories(ludum-dare-client PUBLIC "include/")

target_sources(
  ludum-dare-client
  PRIVATE
    src/button.cpp
    src/camera.cpp
    src/mine.cpp
    src/miner.cpp
    src/overlay.cpp
    src/renderer.cpp
    src/scene.cpp
    src/source.cpp
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

set_target_properties(
  ludum-dare-client
  PROPERTIES
    LINK_FLAGS "--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/../resources@resources"
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
    ${CMAKE_BINARY_DIR}/application/ludum-dare-client.data
  DESTINATION ${CMAKE_INSTALL_BINDIR}
  COMPONENT core
)
