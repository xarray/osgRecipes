# - Find ngpcore
# Find the ngplantcopre includes and libraries
# This module defines
#  NGPCORE_INCLUDE_DIRS, where to find ngpcore folder
#  NGPCORE_LIBRARIES, libraries to link against to use ngpcore.
#  NGPCORE_FOUND, If false, do not try to use ngpcore.
#
# Search hint (in CMake or as an environment variable):
#  NGPCORE_ROOT, the ngpcore install directory root.
#
# Find ngpcore static or dynamic libs? Set as a CMake veriable:
#  NGPCORE_STATIC, find the static libs if ON, dynamic by default



# Find the main libHaru header.
set( NGPCORE_INCLUDE_DIRS )
find_path( NGPCORE_INCLUDE_DIRS ngpcore
    PATHS ${NGPCORE_ROOT} ENV NGPCORE_ROOT
    PATH_SUFFIXES include
)


# Get a list of libraries, with static 's' suffix if necessary.
set( _requestedComponents ngpcore )
set( _components )
foreach( lib ${_requestedComponents} )
    if( NGPCORE_STATIC )
        list( APPEND _components ${lib}s )
    else()
        list( APPEND _components ${lib} )
    endif()
endforeach()

# Find each library.
set( NGPCORE_LIBRARIES )
foreach( lib ${_components} )
    find_library( NGPCORE_${lib}_LIBRARY
        NAMES ${lib}
        PATHS ${NGPCORE_ROOT} ENV NGPCORE_ROOT
        PATH_SUFFIXES lib
    )
    find_library( NGPCORE_${lib}_LIBRARY_DEBUG
        NAMES ${lib}d
        PATHS ${NGPCORE_ROOT} ENV NGPCORE_ROOT
        PATH_SUFFIXES lib
    )

    if( NOT NGPCORE_${lib}_LIBRARY )
        message( WARNING "Could not find NGPCORE component library ${lib}" )
    else()
        if( NGPCORE_${lib}_LIBRARY_DEBUG AND
                ( NOT NGPCORE_${lib}_LIBRARY_DEBUG STREQUAL NGPCORE_${lib}_LIBRARY ) )
            list( APPEND NGPCORE_LIBRARIES "optimized" ${NGPCORE_${lib}_LIBRARY} )
            list( APPEND NGPCORE_LIBRARIES "debug" ${NGPCORE_${lib}_LIBRARY_DEBUG} )
        else()
            list( APPEND NGPCORE_LIBRARIES ${NGPCORE_${lib}_LIBRARY} )
        endif()
        mark_as_advanced( NGPCORE_${lib}_LIBRARY )
        mark_as_advanced( NGPCORE_${lib}_LIBRARY_DEBUG )
    endif()
endforeach()


# handle the QUIETLY and REQUIRED arguments and set NGPCORE_FOUND to TRUE if 
# all listed variables are TRUE
include( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( NGPCORE
    REQUIRED_VARS NGPCORE_INCLUDE_DIRS NGPCORE_LIBRARIES
)


mark_as_advanced(
    ASSIMP_INCLUDE_DIR
    ASSIMP_LIBRARIES
)
