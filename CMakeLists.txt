# ------------------------------------------------------------------------------
# CMAKE SETTINGS
# ------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10)

# Project Metadata
project(wasm-cross VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------------------------------------
# OPTIONS AND GLOBALS
# ------------------------------------------------------------------------------

set(TARGET_NAME ${PROJECT_NAME})
set(APPLICATION_ORGANIZATION "Mhoek")
set(APPLICATION_ORGANIZATION_DOMAIN "https://github.com/mhoek2/")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# ------------------------------------------------------------------------------
# GIT METADATA
# ------------------------------------------------------------------------------

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    execute_process(
        COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 --tags
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_TAG
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(PROJECT_NAME ${TARGET_NAME}-${GIT_BRANCH})
    set(APPLICATION_VERSION ${GIT_TAG})
endif()

# ------------------------------------------------------------------------------
# DEPENDENCIES
# ------------------------------------------------------------------------------

# GLFW
add_subdirectory(external/glfw)

# GLM
include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glm)

# GLEW
set(GLEW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/glew/include)
set(GLEW_SOURCE ${CMAKE_SOURCE_DIR}/external/glew/src/glew.c)

add_library(glew STATIC ${GLEW_SOURCE})
target_include_directories(glew PUBLIC ${GLEW_INCLUDE_DIR})
target_compile_definitions(glew PUBLIC -DGLEW_STATIC)

# ------------------------------------------------------------------------------
# FILES
# ------------------------------------------------------------------------------

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS src/*.h)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.cpp)
file(GLOB_RECURSE SHADER_FILES CONFIGURE_DEPENDS src/glsl/*.vert src/glsl/*.frag)

if(WIN32)
    list(APPEND SOURCE_FILES ${HEADER_FILES}) # For Visual Studio filters
endif()

# ------------------------------------------------------------------------------
# SHADERS
# ------------------------------------------------------------------------------

#add_custom_target(copy_shaders ALL 
#    COMMAND ${CMAKE_COMMAND} -E copy_directory
#        "${CMAKE_SOURCE_DIR}/src/glsl"
#        "$<TARGET_FILE_DIR:${TARGET_NAME}>"
#)

# ------------------------------------------------------------------------------
# INCLUDE DIRECTORIES
# ------------------------------------------------------------------------------

include_directories(
    ${CMAKE_SOURCE_DIR}/includes
    ${GLEW_INCLUDE_DIR}
)

# ------------------------------------------------------------------------------
# EXECUTABLE
# ------------------------------------------------------------------------------

add_executable(${TARGET_NAME}
    ${SOURCE_FILES}
    ${SHADER_FILES}
)

# Link libraries
target_link_libraries(${TARGET_NAME}
    glfw
    glew
    glm
)

if(WIN32)
    target_link_libraries(${TARGET_NAME} opengl32 glu32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${TARGET_NAME} GL)
endif()


# ------------------------------------------------------------------------------
# SHADERS
# ------------------------------------------------------------------------------

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/glsl"
        "$<TARGET_FILE_DIR:${TARGET_NAME}>/glsl"
)

# ------------------------------------------------------------------------------
# ASSETS
# ------------------------------------------------------------------------------

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${TARGET_NAME}>/assets"
)

# ------------------------------------------------------------------------------
# OUTPUT CONFIGURATION
# ------------------------------------------------------------------------------

# Set output directories
foreach(OUTPUTCONFIG IN ITEMS DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_SOURCE_DIR}/build/${OUTPUTCONFIG}")
endforeach()

# For Visual Studio: Set debugger working directory per config (Debug/Release/etc.)
set_target_properties(${TARGET_NAME} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/build/$<CONFIG>"
)

# ------------------------------------------------------------------------------
# INSTALLATION
# ------------------------------------------------------------------------------

install(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
include(InstallRequiredSystemLibraries)

# ------------------------------------------------------------------------------
# SOURCE GROUPS (VISUAL STUDIO)
# ------------------------------------------------------------------------------

source_group("Headers" FILES ${HEADER_FILES})
source_group("Sources" FILES ${SOURCE_FILES})
source_group("Shaders" FILES ${SHADER_FILES})
