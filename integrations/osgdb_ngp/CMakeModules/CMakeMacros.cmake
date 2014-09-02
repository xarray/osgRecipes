macro( _addOSGPlugin TRGTNAME )
    if( BUILD_SHARED_LIBS )
        add_library( ${TRGTNAME} MODULE ${ARGN} )
    else()
        add_library( ${TRGTNAME} STATIC ${ARGN} )
    endif()

    include_directories(
        ${OPENSCENEGRAPH_INCLUDE_DIRS}
        ${NGPCORE_INCLUDE_DIRS}
    )

    target_link_libraries( ${TRGTNAME}
        ${OPENSCENEGRAPH_LIBRARIES}
        ${NGPCORE_LIBRARIES}
    )

    set_target_properties( ${TRGTNAME} PROPERTIES PROJECT_LABEL "Plugin ${TRGTNAME}" )

    set( _libName ${TRGTNAME} )
    include( ModuleInstall REQUIRED )
endmacro()
