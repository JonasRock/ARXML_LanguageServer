cmake_minimum_required(VERSION 3.10)

project(ARXML_LanguageServer VERSION 0.1)

######## Configuration Options ########
# Set the path to the boost/iostreams/src/mapped_file.cpp
set(BOOST_MAPPED_FILE_CPP_PATH C:/boost/libs/iostreams/src/mapped_file.cpp)
# If YES, the server will output messages to the terminal even in release mode
set(ENABLE_RELEASE_CONSOLE_OUTPUT YES)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(GCC_COVERAGE_COMPILE_FLAGS "-fdiagnostics-color")
if (ENABLE_RELEASE_CONSOLE_OUTPUT)
  message("Custom Settings: Terminal Logging enabled")
else()
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -DNO_TERMINAL_OUTPUT")
  message("Custom Settings: ReleasTerminal Logging disabled")
endif()
message("Custom Settings: boost::mapped_file.cpp location: " ${BOOST_MAPPED_FILE_CPP_PATH})

add_executable(ARXML_LanguageServer
  src/main.cpp
  src/config.cpp
  src/xmlParser.cpp
  src/ioHandler.cpp
  src/languageService.cpp
  src/shortnameStorage.cpp
  src/messageParser.cpp
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