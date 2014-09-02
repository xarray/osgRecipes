#
# osgngp top-level CMakeLists.txt
# https://github.com/XenonofArcticus/osgngp
#

cmake_minimum_required( VERSION 2.8.5 )
project( osgdb_ngp )

# Define project-specific macros.
set( CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules ${CMAKE_MODULE_PATH} )
include( CMakeMacros )


# Installation directories
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )


#
# OpenSceneGraph.
# Set OSG_DIR (env or cmake) as hint, if necessary.
set( _osgComponents osgGA osgText osgViewer osgSim osgDB osgUtil osg OpenThreads )
find_package( OpenSceneGraph 2.8.5 COMPONENTS ${_osgComponents} REQUIRED )

#
# require ngpcore.
# Set NGPCORE_ROOT (env or cmake) as hint, if necessary.
find_package( NGPCORE REQUIRED )

#
# By default, build shared libraries.
set( BUILD_SHARED_LIBS ON CACHE BOOL "Build shared or static libs" )

if( WIN32 )
    # Common debug postfix for Windows.
    set( CMAKE_DEBUG_POSTFIX d )
endif()



add_subdirectory( src )



#
# Install pdb files for Debug and RelWithDebInfo builds
if( MSVC )
    install(
        DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\${CMAKE_INSTALL_CONFIG_NAME}/
        DESTINATION lib
        USE_SOURCE_PERMISSIONS
        COMPONENT libprc-dev
        FILES_MATCHING PATTERN "*.pdb"
    )
    install(
        DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/\${CMAKE_INSTALL_CONFIG_NAME}/
        DESTINATION lib
        USE_SOURCE_PERMISSIONS
        COMPONENT libprc-dev
        FILES_MATCHING PATTERN "*.pdb"
    )
endif()
