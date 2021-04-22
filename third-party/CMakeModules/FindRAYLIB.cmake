find_path(
  RAYLIB_INCLUDE_DIR raylib.h
  /usr/include
  /usr/local/include
  /opt/local/include
  ${CMAKE_SOURCE_DIR}/includes
  ${CMAKE_SOURCE_DIR}/dep/raylib/include
  "C:/raylib/include"
)

find_library(
  RAYLIB_LIBRARY raylib
  /usr/lib64
  /usr/lib
  /usr/local/lib
  /opt/local/lib
  ${CMAKE_SOURCE_DIR}/lib
  ${CMAKE_SOURCE_DIR}/dep/raylib/lib
  "C:/raylib/lib"
)

IF(RAYLIB_INCLUDE_DIR AND RAYLIB_LIBRARY)
  set(RAYLIB_FOUND TRUE)
  set(RAYLIB_LIBRARIES ${RAYLIB_LIBRARY})
  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(
    RAYLIB DEFAULT_MSG
    RAYLIB_LIBRARY RAYLIB_INCLUDE_DIR
  )
endif()
