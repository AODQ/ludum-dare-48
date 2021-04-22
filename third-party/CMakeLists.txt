LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules)


find_package(RAYLIB REQUIRED)

if (RAYLIB_FOUND)
  message(STATUS "Found raylib in ${RAYLIB_INCLUDE_DIR} and ${RAYLIB_LIBRARY}")
else()
  message(FATAL_ERROR "could not find raylib library")
endif()
