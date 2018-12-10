cmake_minimum_required(VERSION 3.1)

project(tjtech1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(CMAKE_CXX_FLAGS_DEBUG "-O0")
SET(CMAKE_C_FLAGS_DEBUG "-O0")

set(FILE_TARGET "${PROJECT_NAME}")

set(DIR_ROOT "${CMAKE_CURRENT_SOURCE_DIR}")
set(DIR_SRC "${DIR_ROOT}/src")
set(DIR_VENDOR "${DIR_ROOT}/vendor")
set(DIR_GLFW "${DIR_VENDOR}/glfw")

set(SOURCES "${DIR_SRC}/main.c" "${DIR_SRC}/util.c")

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

set(GLSL_VALIDATOR "$ENV{VULKAN_SDK}/bin/glslangValidator")

file(GLOB_RECURSE GLSL_SOURCE_FILES
    "shaders/*.frag"
    "shaders/*.vert"
    )

foreach(GLSL ${GLSL_SOURCE_FILES})
  get_filename_component(FILE_NAME ${GLSL} NAME)
  set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILE_NAME}.spv")
  add_custom_command(
    OUTPUT ${SPIRV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
    COMMAND ${GLSL_VALIDATOR} -V ${GLSL} -o ${SPIRV}
    DEPENDS ${GLSL})
  list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(
    shaders
    DEPENDS ${SPIRV_BINARY_FILES}
    )

add_dependencies(${FILE_TARGET} shaders)

add_custom_command(TARGET ${FILE_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:${FILE_TARGET}>/shaders/"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${PROJECT_BINARY_DIR}/shaders"
        "$<TARGET_FILE_DIR:${FILE_TARGET}>/shaders"
        )
