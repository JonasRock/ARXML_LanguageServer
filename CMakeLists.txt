cmake_minimum_required(VERSION 3.10)

project(ARXML_LanguageServer VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(ARXML_LanguageServer src/warehouse/main.cpp src/warehouse/warehouseapp.cpp)