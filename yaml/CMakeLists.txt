project(yaml)

cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB SOURCE_FILES "src/*.h" "src/*.cpp")
add_subdirectory(../../yaml-cpp ../../yaml-cpp)
include_directories(BEFORE ../../yaml-cpp/include)
add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} yaml-cpp)
