cmake_minimum_required(VERSION 3.16)
project(FreeLinkUpdater LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(updater WIN32 MACOSX_BUNDLE updater.cpp)

if(WIN32)
    target_link_libraries(updater PRIVATE shell32)
endif()
