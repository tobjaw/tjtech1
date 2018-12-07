cmake_minimum_required(VERSION 3.1)

project(tjtech1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(FILE_TARGET "${PROJECT_NAME}")

set(DIR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(DIR_SRC "${DIR_ROOT}/src")
set(DIR_VENDOR "${DIR_ROOT}/vendor")
set(DIR_GLFW "${DIR_VENDOR}/glfw")

set(SOURCES "${DIR_SRC}/main.c")

add_executable(${FILE_TARGET} ${SOURCES})
target_include_directories(${FILE_TARGET} PRIVATE "${DIR_SRC}")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("${DIR_GLFW}")
target_link_libraries(${FILE_TARGET} "glfw" "${GLFW_LIBRARIES}")
target_include_directories(${FILE_TARGET} PRIVATE "${DIR_GLFW}/include")
target_compile_definitions(${FILE_TARGET} PRIVATE "GLFW_INCLUDE_NONE")

find_package(vulkan REQUIRED)
target_link_libraries(${FILE_TARGET} Vulkan::Vulkan)
