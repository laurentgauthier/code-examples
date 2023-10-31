project(video)

cmake_minimum_required(VERSION 3.0)

include_directories(include)
# Add both src and include to the list of source files
file(GLOB SRC_LIST include/video/*.h src/*.c )
set(INC_LIST )
add_executable(${PROJECT_NAME} ${SRC_LIST} ${INC_LIST})
target_link_libraries(${PROJECT_NAME} )
