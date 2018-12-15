cmake_minimum_required(VERSION 3.6)
cmake_policy(SET CMP0079 NEW)

# project #####################################################################
project(tjtech1
  LANGUAGES C
  VERSION 0.1.0
)
message("${PROJECT_NAME}")
message(STATUS "PROJECT_SOURCE_DIR:  ${PROJECT_SOURCE_DIR}")
message(STATUS "PROJECT_BINARY_DIR:  ${PROJECT_BINARY_DIR}")
message(STATUS "PROJECT_VERSION:     ${PROJECT_VERSION}")

message(STATUS "CMAKE_C_COMPILER_ID: ${CMAKE_C_COMPILER_ID}")
# Build type selection
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  # message("Setting build type to 'Debug' as none was specified.")
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "CMAKE_BUILD_TYPE:    ${CMAKE_BUILD_TYPE}")

# output compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# preface #####################################################################
# interfaces for different compiler error levels
add_library(CompilerErrors_High INTERFACE)
add_library(CompilerErrors::High ALIAS CompilerErrors_High)
target_compile_options(CompilerErrors_High INTERFACE
    $<$<OR:$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:GNU>>:-Wall;-Wextra>
    $<$<C_COMPILER_ID:MSVC>:/W4>
)
add_library(CompilerErrors_Low INTERFACE)
add_library(CompilerErrors::Low ALIAS CompilerErrors_Low)
target_compile_options(CompilerErrors_Low INTERFACE
    $<$<OR:$<C_COMPILER_ID:AppleClang>,$<C_COMPILER_ID:GNU>>:-w>
    $<$<C_COMPILER_ID:MSVC>:/W1>
)
# make interface available for includes
install(
  TARGETS CompilerErrors_Low
  EXPORT CompilerErrors_Low
)
install(
  EXPORT CompilerErrors_Low
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/${SHADER}
)


# src #########################################################################
message("src")
add_executable(tjtech1
  src/main.c
  src/util.c
)
target_link_libraries(tjtech1
  PRIVATE CompilerErrors::High
)


# includes ####################################################################
# GLFW
message("inc/GLFW")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("vendor/glfw")
target_link_libraries(tjtech1
  PUBLIC glfw "${GLFW_LIBRARIES}"
)
target_include_directories(tjtech1 SYSTEM PUBLIC "vendor/glfw/include")
target_compile_definitions(tjtech1 PUBLIC "GLFW_INCLUDE_NONE")
target_link_libraries(glfw
  PUBLIC CompilerErrors::Low
)

# Vulkan
message("inc/Vulkan")
if(NOT DEFINED ENV{VULKAN_SDK})
  message(FATAL_ERROR "VULKAN_SDK not defined.")
else()
  message(STATUS "VULKAN_SDK: $ENV{VULKAN_SDK}")
endif()
find_package(vulkan REQUIRED)
target_link_libraries(tjtech1 PRIVATE Vulkan::Vulkan)


# assets ######################################################################
# compile GLSL shaders into SPIR-V bytecode.
message("assets/shaders")
set(SHADERS
  shaders/shader.vert
  shaders/shader.frag
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/shaders)
find_program(BIN_SPIRV glslangValidator)
if(NOT BIN_SPIRV)
  message(FATAL_ERROR "glslangValidator not found.")
endif()
foreach(SHADER IN LISTS SHADERS)
  add_custom_command(
    OUTPUT ${SHADER}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
            ${CMAKE_CURRENT_BINARY_DIR}/${SHADER}
    COMMAND ${BIN_SPIRV} -V -o ${SHADER}.spv ${SHADER}
    COMMENT "Compiling ${SHADER} to SPIR-V"
    VERBATIM
  )
endforeach()
add_custom_target(shaders DEPENDS ${SHADERS})
add_dependencies(tjtech1 shaders)


# optional ####################################################################
# use clang-tidy
message("opt/clang-tidy")
find_program(BIN_CLANG_TIDY clang-tidy)
if(NOT BIN_CLANG_TIDY)
  message(WARNING "clang-tidy not found")
else()
  set_target_properties(tjtech1 PROPERTIES
    C_CLANG_TIDY "${BIN_CLANG_TIDY}"
  )
endif()
