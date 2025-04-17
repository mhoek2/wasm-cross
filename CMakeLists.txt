#-------------------------------------------------------------------------------
#	CMAKE SETTINGS
#-------------------------------------------------------------------------------

cmake_minimum_required(VERSION 2.8.12)

add_subdirectory(external/glfw)

SET(CMAKE_INCLUDE_CURRENT_DIR ON)



# .h files
list(APPEND HEADER_FILES

)

# .cpp files
list(APPEND SOURCE_FILES
src/main.cpp
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


source_group(source REGULAR_EXPRESSION ".*[.]cpp")
source_group(headers REGULAR_EXPRESSION ".*[.]h")

include_directories(
    ${CMAKE_SOURCE_DIR}/includes
    ${INCLUDE_DIRS}
)

# Generate the executable
add_executable(${TARGET_NAME} ${SOURCE_FILES} ${RESOURCES})
target_link_libraries(${TARGET_NAME} glfw)

if(WIN32)
    target_link_libraries(${TARGET_NAME} opengl32)
elseif(UNIX AND NOT APPLE)
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
endforeach()
