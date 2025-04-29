cmake_minimum_required(VERSION 3.10)

project(cs-s7)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
"${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(CSOUND)
if(NOT CSOUND_FOUND)
      message(FATAL_ERROR "Csound library is required")
endif()

find_library(TECLA tecla HINTS /opt/homebrew/lib /usr/local/lib)


add_executable(cs-s7  ./src/cs-s7.c ./src/main.c ./s7/s7.c)
add_library(css7 SHARED ./src/cs-s7.c ./s7/s7.c)

if(NOT TECLA)
   message(STATUS "libtecla not found")
else()
  message(STATUS "libtecla found")
  target_link_libraries(cs-s7 ${TECLA})
  find_path(TECLA_INCLUDE_DIR libtecla.h HINTS /opt/homebrew/include /usr/local/include)
  target_include_directories(cs-s7 PRIVATE ${TECLA_INCLUDE_DIR})
  target_compile_definitions(cs-s7 PRIVATE USE_TECLA)
endif()

if(APPLE) 
target_link_libraries(cs-s7 ${CSOUND_FRAMEWORK})
else()
target_link_libraries(cs-s7 ${CSOUND_LIBRARY})
endif()
target_include_directories(cs-s7 PRIVATE ./src ./s7)
target_include_directories(css7 PRIVATE ./src ./s7 ${CSOUND_INCLUDE_DIRS})
target_compile_definitions(css7 PRIVATE BUILDING_MODULE)
if(APPLE)
target_compile_options(css7 PRIVATE -F$ENV{HOME}/Library/frameworks -F/Library/frameworks)
endif()
