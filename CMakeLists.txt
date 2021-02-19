cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ARXML_LanguageServer VERSION 0.1)

add_executable(ARXML_LanguageServer
    src/main.cpp
    src/config.cpp
    src/xmlParser.cpp
    src/ioHandler.cpp
    src/languageService.cpp
    src/arxmlStorage.cpp
    src/messageParser.cpp
)

if(MSVC)
    target_compile_options(ARXML_LanguageServer PUBLIC /std:c++17 /nr:false /o2)
elseif(CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(ARXML_LanguageServer PUBLIC -static -Wall -Wextra)
endif()
    
if(WIN32)
    target_link_libraries(ARXML_LanguageServer PUBLIC ws2_32 wsock32 winpthread)
else()
    target_link_libraries(ARXML_LanguageServer PUBLIC pthread)
endif()

target_include_directories(ARXML_LanguageServer PUBLIC include)
target_include_directories(ARXML_LanguageServer PUBLIC include/extern)
add_subdirectory(src)
add_subdirectory(include)
add_subdirectory(include/extern)

set(Boost_ARCHITECTURE "-x64")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
find_package(Boost 1.71.0 REQUIRED COMPONENTS filesystem iostreams)
target_link_libraries(ARXML_LanguageServer PUBLIC Boost::filesystem Boost::iostreams)