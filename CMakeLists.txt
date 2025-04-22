#-------------------------------------------------------------------------------
#	CMAKE SETTINGS
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.10)

add_subdirectory(external/glfw)
SET(CMAKE_INCLUDE_CURRENT_DIR ON)



include(FetchContent)
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG master
)
FetchContent_MakeAvailable(glm)


# .h files
list(APPEND HEADER_FILES
src/local.h
)

# .cpp files
list(APPEND SOURCE_FILES
src/main.cpp
src/shader.cpp
src/frame.cpp
src/framebuffer.cpp
)



# on windows, append headers to source so they show up in visual studio
if(WIN32)
	LIST(APPEND SOURCE_FILES ${HEADER_FILES})
endif()

#-------------------------------------------------------------------------------
#	PROJECT SETTINGS
#-------------------------------------------------------------------------------

set(TARGET_NAME wasm-cross)

# Use git branch as project name if clones from git
find_package(Git)
if(GIT_FOUND)
	execute_process(
	  COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
	  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	  OUTPUT_VARIABLE BRANCH
	  OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	execute_process(
	  COMMAND ${GIT_EXECUTABLE} describe --abbrev=0 --tags
	  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
	  OUTPUT_VARIABLE VERSION
	  OUTPUT_STRIP_TRAILING_WHITESPACE
	)
	set(PROJECT_NAME ${TARGET_NAME}-${BRANCH})
else()
	set(PROJECT_NAME ${TARGET_NAME})
endif()

# Set the project name
PROJECT(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)

if(MSVC)
	add_compile_options(/MP)
	add_compile_options(/std:c++17)
elseif(CMAKE_COMPILER_IS_GNUCXX)
	add_compile_options(-std=c++17)
endif(MSVC)


set(APPLICATION_NAME ${TARGET_NAME})
set(APPLICATION_VERSION ${VERSION})
set(APPLICATION_ORGANIZATION "Mhoek")
set(APPLICATION_ORGANIZATION_DOMAIN "https://github.com/mhoek2/")

file(GLOB Shaders "${CMAKE_SOURCE_DIR}/src/glsl/*.vert" "${CMAKE_SOURCE_DIR}/src/glsl/*.frag")
source_group("glsl" FILES ${Shaders})

foreach(SHADER_FILE ${Shaders})
    get_filename_component(SHADER_NAME ${SHADER_FILE} NAME)
    configure_file(${SHADER_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME} COPYONLY)
endforeach()

source_group(source REGULAR_EXPRESSION ".*[.]cpp")
source_group(headers REGULAR_EXPRESSION ".*[.]h")


#set(GLEW_LIB_PATH "${CMAKE_SOURCE_DIR}/includes/glew/lib/Release/Win32")
#set(GLEW_DLL_PATH "${CMAKE_SOURCE_DIR}/includes/glew/bin/Release/Win32/glew32.dll")
set(GLEW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/glew/include)
set(GLEW_SOURCE ${CMAKE_SOURCE_DIR}/external/glew/src/glew.c)
message(STATUS "GLEW_INCLUDE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "GLEW_SOURCE: ${CMAKE_SOURCE_DIR}")

add_definitions(-DGLEW_STATIC) 

include_directories(
    ${CMAKE_SOURCE_DIR}/includes
    ${GLEW_INCLUDE_DIR}
    ${INCLUDE_DIRS}
)

add_library(glew STATIC ${GLEW_SOURCE})
target_include_directories(glew PUBLIC ${GLEW_INCLUDE_DIR})

#link_directories(${GLEW_LIB_PATH})

# Generate the executable
add_executable(${TARGET_NAME} ${Shaders} ${SOURCE_FILES} ${RESOURCES})
target_link_libraries(${TARGET_NAME} glfw )

if(WIN32)
    target_link_libraries(${TARGET_NAME} glew opengl32 glu32 glm)
elseif(UNIX AND NOT APPLE)
	list(APPEND LIBRARIES
		${CMAKE_SOURCE_DIR}/includes/glew/lib/Release/x64
	)
	target_link_libraries(${PROJECT_NAME} glew)
    target_link_libraries(${TARGET_NAME} GL)
endif()


# link libraries
target_link_libraries(${TARGET_NAME} ${LIBRARIES})

# Use the automatically determined RPATH values
set_target_properties(${TARGET_NAME} PROPERTIES INSTALL_RPATH_USE_LINK_PATH TRUE)

INSTALL(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin)
INCLUDE(InstallRequiredSystemLibraries)

# Set the build output directory to ./build
foreach(OUTPUTCONFIG DEBUG RELEASE RELWITHDEBINFO MINSIZEREL)
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG_UPPER)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER} "${CMAKE_SOURCE_DIR}/build/${OUTPUTCONFIG}")

	#add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
	#    COMMAND ${CMAKE_COMMAND} -E copy_if_different 
	#    ${GLEW_DLL_PATH} 
	#    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG_UPPER}}/glew32.dll
	#)
endforeach()

