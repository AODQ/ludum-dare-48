add_executable(ludum-dare-client)

target_sources(
  ludum-dare-client
  PRIVATE
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
    raylib
)

install(
  TARGETS ludum-dare-client
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT core
)
