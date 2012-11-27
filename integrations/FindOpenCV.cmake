# Find OpenCV Library
#
# Input variables:
# - OpenCV_ROOT_DIR:          (optional) Force a certain version of the OpenCV library.
#                             When not specified, a best guess will be made.
#
# Output variables:
# - OpenCV_FOUND:             True if OpenCV library was found properly and is ready to be used.
# - OpenCV_INCLUDE_DIRS:      Include directories for OpenCV library.
# - OpenCV_LIBRARIES_RELEASE: Release version of OpenCV libraries.
# - OpenCV_LIBRARIES_DEBUG:   Debug version of OpenCV libraries.
# - OpenCV_LIBRARIES:         Release and debug version of OpenCV libraries.

# create possible root directory list
if(WIN32)
    if(MSVC_VERSION MATCHES "^1400$")
        set(_compiler vs2005)
    elseif(MSVC_VERSION MATCHES "^1500$")
        set(_compiler vs2008)
    elseif(MSVC_VERSION MATCHES "^1600$")
        set(_compiler vs2010)
    endif(MSVC_VERSION MATCHES "^1400$")
else(WIN32)
    execute_process(COMMAND ${CMAKE_C_COMPILER} "-dumpversion" RESULT_VARIABLE _result OUTPUT_VARIABLE _output)
    if(NOT _result AND _output)
        string(REGEX REPLACE "^\([0-9]+\)\\.\([0-9]+\)\\.[0-9]+[^0-9]*" "\\1\\2" _version "${_output}")
    endif(NOT _result AND _output)
    set(_compiler "gcc${_version}")
endif(WIN32)
if(WIN32)
    if(CMAKE_CL_64)
        set(_bitness 64)
    else(CMAKE_CL_64)
        set(_bitness 32)
    endif(CMAKE_CL_64)
else(WIN32)
    if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "64$")
        set(_bitness 64)
    else(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "64$")
        set(_bitness 32)
    endif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "64$")
endif(WIN32)
get_filename_component(_registry_v2.4.0 "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OpenCV 2.4.0 win${_bitness} ${_compiler};UninstallString]" ABSOLUTE)
string(REGEX REPLACE "(.*)/Uninstall.exe" "\\1" _registry_v2.4.0 ${_registry_v2.4.0})
set(OpenCV_POSSIBLE_ROOT_DIRS
    ${OpenCV_POSSIBLE_ROOT_DIRS}
    $ENV{OPENCV}
    $ENV{OPENCV_ROOT}
    # windows
    ${_registry_v2.4.0}
    "C:/OpenCV 2.4.0 win${_bitness} ${_compiler}"
    "$ENV{ProgramFiles}/OpenCV 2.4.0 win${_bitness} ${_compiler}"
    # linux
    $ENV{OPENCV_ROOT}/OpenCV-2.4.0-linux${_bitness}-${_compiler}
    $ENV{OPENCV_ROOT}/opencv-2.4.0-linux${_bitness}-${_compiler}
    /usr/local
    /usr
    /opt/local
    /opt
)

# create components list
set(OpenCV_COMPONENTS "core")
foreach(_component ${OpenCV_FIND_COMPONENTS})
    list(APPEND OpenCV_COMPONENTS ${_component})
endforeach()
list(REMOVE_DUPLICATES OpenCV_COMPONENTS)

# find root directory if necessary
if(NOT OpenCV_ROOT_DIR)
    find_path(OpenCV_ROOT_DIR
        NAMES include/opencv2/opencv.hpp
        PATHS ${OpenCV_POSSIBLE_ROOT_DIRS}
    )
endif(NOT OpenCV_ROOT_DIR)

# set include directory
set(OpenCV_INCLUDE_DIRS
    ${OpenCV_ROOT_DIR}/include
    ${OpenCV_ROOT_DIR}/include/opencv
)

# find library for each component
set(OpenCV_LIBRARIES_RELEASE)
set(OpenCV_LIBRARIES_DEBUG)
foreach(_component ${OpenCV_COMPONENTS})
    string(TOLOWER ${_component} _name)

    # set path and postfix
    if(WIN32)
        set(_version 240) # hack to find the libs, set this to the version you are using
    else(WIN32)
        set(_version)
    endif(WIN32)

    # find release and debug library
    find_library(OpenCV_${_name}_LIBRARY_RELEASE
        NAMES opencv_${_name}${_version}
        PATHS ${OpenCV_ROOT_DIR}
        PATH_SUFFIXES lib
        NO_DEFAULT_PATH
    )
    find_library(OpenCV_${_name}_LIBRARY_DEBUG
        NAMES opencv_${_name}${_version}d
        PATHS ${OpenCV_ROOT_DIR}
        PATH_SUFFIXES lib
        NO_DEFAULT_PATH
    )

    # mark as advanced
    mark_as_advanced(OpenCV_${_name}_LIBRARY_RELEASE)
    mark_as_advanced(OpenCV_${_name}_LIBRARY_DEBUG)

    # append to lists
    list(APPEND OpenCV_LIBRARIES_RELEASE ${OpenCV_${_name}_LIBRARY_RELEASE})
    list(APPEND OpenCV_LIBRARIES_DEBUG ${OpenCV_${_name}_LIBRARY_DEBUG})
endforeach(_component)

# create cmake compatible library list
set(OpenCV_LIBRARIES)
foreach(_library ${OpenCV_LIBRARIES_RELEASE})
    list(APPEND OpenCV_LIBRARIES optimized ${_library})
endforeach(_library ${OpenCV_LIBRARIES_RELEASE})
foreach(_library ${OpenCV_LIBRARIES_DEBUG})
    list(APPEND OpenCV_LIBRARIES debug ${_library})
endforeach(_library ${OpenCV_LIBRARIES_DEBUG})

# handle required and quiet parameters
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(OpenCV DEFAULT_MSG
    OpenCV_ROOT_DIR
    OpenCV_INCLUDE_DIRS
    OpenCV_LIBRARIES_RELEASE
    OpenCV_LIBRARIES_DEBUG
    OpenCV_LIBRARIES
)
set(OpenCV_FOUND ${OPENCV_FOUND})
