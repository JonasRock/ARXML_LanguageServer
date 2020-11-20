cmake_minimum_required(VERSION 3.10)

project(ARXML_LanguageServer VERSION 0.1)

######## Configuration Options ########
# If YES, the server will output messages to the terminal even in release mode
set(ENABLE_RELEASE_CONSOLE_OUTPUT YES)
# Set your boost locations
set(BOOST_ROOT)
set(BOOST_LIBRARYDIR C:/boost/boost_1_74_0/stage/lib)
set(BOOST_INCLUDEDIR C:/boost/boost_1_74_0/)
set(Boost_ARCHITECTURE "-x64")

######## Output ########
if (ENABLE_RELEASE_CONSOLE_OUTPUT)
  message("Custom Settings: Terminal Logging enabled")
else()
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_DEBUG} -DNO_TERMINAL_OUTPUT")
  message("Custom Settings: ReleasTerminal Logging disabled")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CXX_STANDARD_LIBRARIES}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Bstatic,--whole-archive -lwinpthread -Wl,--no-whole-archive")
set(BUILD_SHARED_LIBS OFF)


add_executable(ARXML_LanguageServer
  src/main.cpp
  src/config.cpp
  src/xmlParser.cpp
  src/ioHandler.cpp
  src/languageService.cpp
  src/arxmlStorage.cpp
  src/messageParser.cpp
  )
  
  target_include_directories(ARXML_LanguageServer PUBLIC include)
  target_include_directories(ARXML_LanguageServer PUBLIC include/extern)
  add_subdirectory(src)
  add_subdirectory(include)
  add_subdirectory(include/extern)
  target_link_libraries(ARXML_LanguageServer PUBLIC ws2_32 -static)

  # Boost
  set(Boost_USE_STATIC_LIBS ON)
  set(Boost_USE_MULTITHREADED ON)
  set(Boost_USE_STATIC_RUNTIME OFF)
  find_package(Boost 1.74.0 REQUIRED COMPONENTS filesystem iostreams)
  target_link_libraries(ARXML_LanguageServer PUBLIC Boost::filesystem Boost::iostreams)