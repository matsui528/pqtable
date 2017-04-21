cmake_minimum_required(VERSION 2.6)
project(pqtable)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -Wall")

include_directories(src src/sparse_hashtable)

file(GLOB SOURCES src/* src/sparse_hashtable/* )

find_package(OpenCV REQUIRED)
find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

file(GLOB EXAMPLES examples/*.cpp)
foreach(EXAMPLE ${EXAMPLES})
  MESSAGE("TARGET:" ${EXAMPLE})
  get_filename_component(PREFIX ${EXAMPLE} NAME_WE)
  add_executable(${PREFIX} ${EXAMPLE} ${SOURCES})
  target_link_libraries(${PREFIX} ${OpenCV_LIBS})
  target_link_libraries(${PREFIX} tcmalloc)
endforeach(EXAMPLE)

