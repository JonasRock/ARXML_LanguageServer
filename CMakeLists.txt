cmake_minimum_required(VERSION 3.10)

project(ARXML_LanguageServer VERSION 0.1)

######## configuration Options ########
# Set the path to the boost/iostreams/src/mapped_file.cpp
set(BOOST_MAPPED_FILE_CPP_PATH C:/boost/libs/iostreams/src/mapped_file.cpp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(GCC_COVERAGE_COMPILE_FLAGS "-fdiagnostics-color")

add_executable(ARXML_LanguageServer
  src/main.cpp
  src/config.cpp
  src/xmlParser.cpp
  src/ioHandler.cpp
  src/languageService.cpp
  src/shortnameStorage.cpp
  src/lspParser.cpp
  ${BOOST_MAPPED_FILE_CPP_PATH}
)
target_include_directories(ARXML_LanguageServer PUBLIC include)
target_include_directories(ARXML_LanguageServer PUBLIC include/extern)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(include/extern)

find_package(Boost 1.74.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(ARXML_LanguageServer ws2_32)
endif()