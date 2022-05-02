cmake_minimum_required(VERSION 2.8)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lpthread -std=c++0x")
set(PROJECT_NAME snoop)
set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(SOURCE_FILES ${SOURCE_DIR}/main.cpp)

project(${PROJECT_NAME})
include_directories(SOURCE_DIR)

add_executable(${PROJECT_NAME} ${SOURCE_FILES} )