project(options)

cmake_minimum_required(VERSION 3.0)

include_directories(include)
file(GLOB SOURCE_FILES include/*/*.h src/*.c)
set(INCLUDE_FILES )

add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${INCLUDE_FILES})
target_link_libraries(${PROJECT_NAME} -lrt)
