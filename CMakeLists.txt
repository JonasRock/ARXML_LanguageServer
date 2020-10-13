cmake_minimum_required(VERSION 3.10)

project(ARXML_LanguageServer VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
include_directories(include)

find_package(Boost 1.74.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  add_executable(ARXML_LanguageServer src/main.cpp src/parseHeader.cpp)
  target_link_libraries(ARXML_LanguageServer ws2_32)
endif()


