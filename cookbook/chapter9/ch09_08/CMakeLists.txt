#/**********************************************************\ 
# 
# Auto-generated CMakeLists.txt for the osgWeb project
#
#\**********************************************************/

# Written to work with cmake 2.6
cmake_minimum_required (VERSION 2.6)
set (CMAKE_BACKWARDS_COMPATIBILITY 2.6)

Project(${PLUGIN_NAME})

file (GLOB GENERAL RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    [^.]*.cpp
    [^.]*.h
    [^.]*.cmake
    )

include_directories(${PLUGIN_INCLUDE_DIRS})

# Generated files are stored in ${GENERATED} by the project configuration
SET_SOURCE_FILES_PROPERTIES(
    ${GENERATED}
    PROPERTIES
        GENERATED 1
    )

SOURCE_GROUP(Generated FILES
    ${GENERATED}
    )

SET( SOURCES
    ${GENERAL}
    ${GENERATED}
    )

# This will include Win/projectDef.cmake, X11/projectDef.cmake, Mac/projectDef 
# depending on the platform

find_path( OPENSCENEGRAPH_ROOT include/osg/Node PATHS $ENV{OSG_ROOT} )
include_directories( ${OPENSCENEGRAPH_ROOT}/include )
link_directories( ${Boost_LIBRARY_DIRS} ${OPENSCENEGRAPH_ROOT}/lib )
include_platform()
target_link_libraries(${PROJECT_NAME} OpenThreads osg osgDB osgGA osgViewer)
