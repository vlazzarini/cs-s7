cmake_minimum_required(VERSION 3.5)

project(cs-s7)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH}
"${CMAKE_SOURCE_DIR}/cmake/Modules/")

find_package(CSOUND)
if(NOT CSOUND_FOUND)
      message(FATAL_ERROR "Csound library is required")
endif()

add_executable(cs-s7  ./src/interface.c ./src/main.c ./s7/s7.c)

if(APPLE) 
target_link_libraries(cs-s7 ${CSOUND_FRAMEWORK})
else()
target_link_libraries(cs-s7 ${CSOUND_LIBRARY})
endif()
target_include_directories(cs-s7 PRIVATE ./src ./s7)



