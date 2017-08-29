project(benchmark)

cmake_minimum_required(VERSION 2.8)

include_directories(include)
file(GLOB SOURCE_FILES include/benchmark/*.h src/*.c)
set(INCLUDE_FILES )

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${INCLUDE_FILES})
target_link_libraries(${PROJECT_NAME} -lrt)
