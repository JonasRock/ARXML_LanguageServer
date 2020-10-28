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
  src/xmlParser.cpp
  src/methods.cpp
  src/readWrite.cpp
  src/configurationGlobals.cpp
  src/shortnameStorage.cpp
  ${BOOST_MAPPED_FILE_CPP_PATH}
)
target_include_directories(ARXML_LanguageServer PUBLIC include)
target_include_directories(ARXML_LanguageServer PUBLIC include/thirdparty)

add_executable(test_ARXML
  src/test/testMain.cpp
  src/test/tests.cpp
  src/xmlParser.cpp
  src/methods.cpp
  src/readWrite.cpp
  src/configurationGlobals.cpp
  src/shortnameStorage.cpp
  ${BOOST_MAPPED_FILE_CPP_PATH}
)
target_include_directories(test_ARXML PUBLIC include)
target_include_directories(test_ARXML PUBLIC include/thirdparty)

add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(include/thirdparty)

find_package(Boost 1.74.0 REQUIRED)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  target_link_libraries(ARXML_LanguageServer ws2_32)
  target_link_libraries(test_ARXML ws2_32)
endif()

target_compile_options(test_ARXML PRIVATE -Wa,-mbig-obj)