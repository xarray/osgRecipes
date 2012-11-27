# Find ALVAR Library
#
# Input variables:
# - ALVAR_ROOT_DIR:          (optional) Force a certain version of the ALVAR library.
#                            When not specified, a best guess will be made, prefering the latest revision.
#
# Output variables:
# - ALVAR_FOUND:             True if ALVAR library was found properly and is ready to be used.
# - ALVAR_INCLUDE_DIRS:      Include directories for ALVAR library.
# - ALVAR_LIBRARIES_RELEASE: Release version of ALVAR libraries.
# - ALVAR_LIBRARIES_DEBUG:   Debug version of ALVAR libraries.
# - ALVAR_LIBRARIES:         Release and debug version of ALVAR libraries.

# compiler version macro
macro(compiler_version _version)
    if(WIN32)
        file(WRITE "${CMAKE_BINARY_DIR}/return0.cpp"
            "#include <iostream>\n"
            "int main() {\n"
            "  std::cout << _MSC_VER << std::endl;\n"
            "  return 0;\n"
            "}\n"
        )
        try_run(_run_result _compile_result
            "${CMAKE_BINARY_DIR}" "${CMAKE_BINARY_DIR}/return0.cpp"
            RUN_OUTPUT_VARIABLE _output)
        file(REMOVE "${CMAKE_BINARY_DIR}/return0.cpp")
        if(NOT _run_result AND _compile_result AND _output)
            string(REGEX REPLACE "^\([0-9][0-9][0-9][0-9]\).*" "\\1" _version "${_output}")
        endif(NOT _run_result AND _compile_result AND _output)
        if(_version MATCHES "^1400$")
            set(_version vs2005)
        elseif(_version MATCHES "^1500$")
            set(_version vs2008)
        elseif(_version MATCHES "^1600$")
            set(_version vs2010)
        endif(_version MATCHES "^1400$")
    else(WIN32)
        execute_process(COMMAND ${CMAKE_C_COMPILER} "-dumpversion" RESULT_VARIABLE _result OUTPUT_VARIABLE _output)
        if(NOT _result AND _output)
            string(REGEX REPLACE "^\([0-9]+\)\\.\([0-9]+\)\\.[0-9]+" "\\1\\2" _version "${_output}")
        endif(NOT _result AND _output)
        set(_version "gcc${_version}")
    endif(WIN32)
endmacro(compiler_version)

macro(alvar_compiler _path _compiler)
    file(GLOB _filename "${_path}/build/generate_*")
    if(_filename)
        list(GET _filename 0 _filename)
        string(REGEX REPLACE ".*generate_\([a-z0-9]+\)\\.(bat|sh)" "\\1" ${_compiler} ${_filename})
    else(_filename)
        set(${_compiler} FALSE)
    endif(_filename)
endmacro(alvar_compiler)

macro(alvar_version _path _version)
    if(WIN32)
        set(_library_prefix)
        set(_library_extension lib)
        set(_library_extension_static lib)
    else(WIN32)
        set(_library_prefix lib)
        set(_library_extension so)
        set(_library_extension_static a)
    endif(WIN32)
    file(GLOB _filename
        "${_path}/bin/${_library_prefix}alvar*.${_library_extension}"
        "${_path}/bin/${_library_prefix}alvar*.${_library_extension_static}"
    )
    list(SORT _filename)
    if(_filename)
        list(GET _filename 0 _filename)
        string(REGEX REPLACE ".*/${_library_prefix}alvar([0-9]+(a[0-9]+|b[0-9]+|rc[0-9]+)?(git[a-f0-9][a-f0-9][a-f0-9][a-f0-a][a-f0-9][a-f0-9][a-f0-9])?)d?.(${_library_extension}|${_library_extension_static})" "\\1" ${_version} ${_filename})
    else(_filename)
        set(${_version} FALSE)
    endif(_filename)
endmacro(alvar_version)

# define variables
file(GLOB _alvar_paths "$ENV{ProgramFiles}/*alvar*")
set(ALVAR_POSSIBLE_ROOT_DIRS
    ${ALVAR_POSSIBLE_ROOT_DIRS}
    $ENV{ALVAR}
    $ENV{ALVAR_ROOT}
    ${_alvar_paths}
)
set(ALVAR_INCLUDE_SUFFIXES
    include
    include/platform
)
set(ALVAR_LIBRARY_SUFFIXES
    bin
)
set(ALVAR_COMPONENT_NAMES
    basic
    platform
)
set(ALVAR_COMPONENT_HEADERS
    Alvar.h
    Platform.h
)
set(ALVAR_COMPONENT_LIBRARIES
    alvar
    alvarplatform
)

# determine compiler version
compiler_version(_compiler)

# best attempt at finding alvar root directory
if(ALVAR_ROOT_DIR)
    alvar_compiler(${ALVAR_ROOT_DIR} _alvar_compiler)
    if(NOT ${_compiler} MATCHES ${_alvar_compiler})
        message(FATAL_ERROR "Forced version of ALVAR (ALVAR_ROOT_DIR) not compatible with compiler")
    endif(NOT ${_compiler} MATCHES ${_alvar_compiler})
else(ALVAR_ROOT_DIR)
    set(_root_dirs)
    foreach(_path ${ALVAR_POSSIBLE_ROOT_DIRS})
        #message(STATUS "  ${_path}")
        find_path(_root_dir NAMES include/Alvar.h PATHS "${_path}")
        if(_root_dir)
            alvar_compiler(${_root_dir} _alvar_compiler)
            if(${_compiler} MATCHES ${_alvar_compiler})
                alvar_version(${_root_dir} _alvar_version)
                if(_alvar_version)
                    message(STATUS "Detected ALVAR: ${_root_dir}")
                    list(APPEND _root_dirs "${_alvar_version}-${_root_dir}")
                else(_alvar_version)
                    #message(STATUS "  version not found")
                endif(_alvar_version)
            else(${_compiler} MATCHES ${_alvar_compiler})
                #message(STATUS "  compilers don't match")
            endif(${_compiler} MATCHES ${_alvar_compiler})
        else(_root_dir)
            #message(STATUS "  root dir not found")
        endif(_root_dir)
        unset(_root_dir CACHE)
    endforeach(_path ${ALVAR_POSSIBLE_ROOT_DIRS})

    if(_root_dirs)
        list(SORT _root_dirs)
        list(REVERSE _root_dirs) # prefer latest version
        list(GET _root_dirs 0 _root_dir)
        string(REGEX REPLACE "[0-9]+\\-\(.+\)" "\\1" _root_dir ${_root_dir})
        set(ALVAR_ROOT_DIR ${_root_dir})
    endif(_root_dirs)
endif(ALVAR_ROOT_DIR)
set(ALVAR_ROOT_DIR ${ALVAR_ROOT_DIR} CACHE PATH "")

# determine alvar version
if(ALVAR_ROOT_DIR)
    alvar_version(${ALVAR_ROOT_DIR} ALVAR_VERSION)
endif(ALVAR_ROOT_DIR)

# find include directory and library for each component
set(ALVAR_INCLUDE_DIRS)
set(ALVAR_LIBRARIES)
set(ALVAR_LIBRARIES_DEBUG)
list(LENGTH ALVAR_COMPONENT_HEADERS _length)
math(EXPR _length "${_length} - 1")
foreach(_index RANGE ${_length})
    # get variables
    list(GET ALVAR_COMPONENT_NAMES ${_index} _name)
    list(GET ALVAR_COMPONENT_HEADERS ${_index} _header)
    list(GET ALVAR_COMPONENT_LIBRARIES ${_index} _library)

    # find include directory
    find_path(ALVAR_${_name}_INCLUDE_DIR
        NAMES ${_header}
        PATHS ${ALVAR_ROOT_DIR}
        PATH_SUFFIXES ${ALVAR_INCLUDE_SUFFIXES}
    )

    # find release and debug library
    find_library(ALVAR_${_name}_LIBRARY_RELEASE
        NAMES ${_library}${ALVAR_VERSION}
        PATHS ${ALVAR_ROOT_DIR}
        PATH_SUFFIXES ${ALVAR_LIBRARY_SUFFIXES}
    )
    find_library(ALVAR_${_name}_LIBRARY_DEBUG
        NAMES ${_library}${ALVAR_VERSION}d
        PATHS ${ALVAR_ROOT_DIR}
        PATH_SUFFIXES ${ALVAR_LIBRARY_SUFFIXES}
    )

    # mark as advanced
    mark_as_advanced(ALVAR_${_name}_INCLUDE_DIR)
    mark_as_advanced(ALVAR_${_name}_LIBRARY_RELEASE)
    mark_as_advanced(ALVAR_${_name}_LIBRARY_DEBUG)

    # append to lists
    list(APPEND ALVAR_INCLUDE_DIRS ${ALVAR_${_name}_INCLUDE_DIR})
    list(APPEND ALVAR_LIBRARIES_RELEASE ${ALVAR_${_name}_LIBRARY_RELEASE})
    list(APPEND ALVAR_LIBRARIES_DEBUG ${ALVAR_${_name}_LIBRARY_DEBUG})
endforeach(_index RANGE ${_length})

# create cmake compatible library list
set(ALVAR_LIBRARIES)
foreach(_library ${ALVAR_LIBRARIES_RELEASE})
    list(APPEND ALVAR_LIBRARIES optimized ${_library})
endforeach(_library ${ALVAR_LIBRARIES_RELEASE})
foreach(_library ${ALVAR_LIBRARIES_DEBUG})
    list(APPEND ALVAR_LIBRARIES debug ${_library})
endforeach(_library ${ALVAR_LIBRARIES_DEBUG})

# handle required and quiet parameters
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(ALVAR DEFAULT_MSG
    ALVAR_ROOT_DIR
    ALVAR_INCLUDE_DIRS
    ALVAR_LIBRARIES_RELEASE
    ALVAR_LIBRARIES_DEBUG
    ALVAR_LIBRARIES
)
set(ALVAR_FOUND ${ALVAR_FOUND})
