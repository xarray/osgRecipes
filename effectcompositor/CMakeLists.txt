IF(WIN32)
    IF(MSVC)
        ADD_DEFINITIONS(-D_SCL_SECURE_NO_WARNINGS)
        ADD_DEFINITIONS(-D_CRT_SECURE_NO_DEPRECATE)
    ENDIF(MSVC)
ELSE(WIN32)
    SET(CMAKE_CXX_FLAGS "-W -Wall -Wno-unused")
ENDIF(WIN32)

FIND_PATH(OPENSCENEGRAPH_INCLUDE_DIR osg/Referenced
    PATHS
    $ENV{OSG_ROOT}/include
    $ENV{OSG_DIR}/include
    /usr/include 
    /usr/local/include
)

FIND_PATH(OPENSCENEGRAPH_LIB_DIR libosg.so osg.lib
    PATHS
    $ENV{OSG_ROOT}/lib
    $ENV{OSG_DIR}/lib
    /usr/lib 
    /usr/local/lib
)

INCLUDE_DIRECTORIES(${OPENSCENEGRAPH_INCLUDE_DIR})
LINK_DIRECTORIES(${OPENSCENEGRAPH_LIB_DIR})

MACRO(START_EXAMPLE)

    INCLUDE_DIRECTORIES(${EXTERNAL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    ADD_EXECUTABLE(${EXAMPLE_NAME} ${EXAMPLE_FILES})
    SET_TARGET_PROPERTIES(${EXAMPLE_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
    
    TARGET_LINK_LIBRARIES(${EXAMPLE_NAME}
        debug osg${CMAKE_DEBUG_POSTFIX}          optimized osg
        debug osgAnimation${CMAKE_DEBUG_POSTFIX} optimized osgAnimation
        debug osgParticle${CMAKE_DEBUG_POSTFIX}  optimized osgParticle
        debug osgShadow${CMAKE_DEBUG_POSTFIX}    optimized osgShadow
        debug osgDB${CMAKE_DEBUG_POSTFIX}        optimized osgDB
        debug osgGA${CMAKE_DEBUG_POSTFIX}        optimized osgGA
        debug osgText${CMAKE_DEBUG_POSTFIX}      optimized osgText
        debug osgUtil${CMAKE_DEBUG_POSTFIX}      optimized osgUtil
        debug osgViewer${CMAKE_DEBUG_POSTFIX}    optimized osgViewer
        debug OpenThreads${CMAKE_DEBUG_POSTFIX}  optimized OpenThreads
        ${EXTERNAL_LIBRARIES}
    )
    INSTALL(TARGETS ${EXAMPLE_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ENDMACRO(START_EXAMPLE)

SET(EXAMPLE_NAME osgeffectcompositor)
SET(EXAMPLE_FILES
    osgeffectcompositor.cpp
    SkyBox.cpp
    EffectAnalysisHelper.cpp
    EffectCompositor.cpp
    EffectCompositorXML.cpp
)

FIND_PACKAGE(OpenGL)
FIND_PATH(SILVERLINING_INCLUDE_DIR SilverLining.h)
FIND_LIBRARY(SILVERLINING_LIBRARY NAMES SilverLining-MT-DLL)

OPTION(USE_SILVERLINING "Use SilverLining for sky/cloud rendering" OFF)
IF(USE_SILVERLINING)
    ADD_DEFINITIONS(-DUSE_SILVERLINING)
ENDIF(USE_SILVERLINING)

IF(SILVERLINING_INCLUDE_DIR AND SILVERLINING_LIBRARY)
    
    SET(EXTERNAL_INCLUDE_DIR ${SILVERLINING_INCLUDE_DIR})
    SET(EXTERNAL_LIBRARIES ${SILVERLINING_LIBRARY} ${OPENGL_LIBRARIES})
    SET(SILVERLINING_RECIPE_DIR ${CMAKE_SOURCE_DIR}/integrations/osgsilverlining)
    
    INCLUDE_DIRECTORIES(${SILVERLINING_RECIPE_DIR})
    SET(EXAMPLE_FILES ${EXAMPLE_FILES}
        ${SILVERLINING_RECIPE_DIR}/SilverLiningNode.cpp)
    ADD_DEFINITIONS(-DHAVE_SILVERLINING)
    START_EXAMPLE()
    
ELSE(SILVERLINING_INCLUDE_DIR AND SILVERLINING_LIBRARY)
    
    START_EXAMPLE()
    
ENDIF(SILVERLINING_INCLUDE_DIR AND SILVERLINING_LIBRARY)

# osgFXEffectCompositor library.
ADD_LIBRARY(osgFXEffectCompositor SHARED EffectCompositor.cpp
                                         EffectCompositorXML.cpp)

TARGET_LINK_LIBRARIES(osgFXEffectCompositor
    debug osg${CMAKE_DEBUG_POSTFIX}          optimized osg
    debug osgAnimation${CMAKE_DEBUG_POSTFIX} optimized osgAnimation
    debug osgParticle${CMAKE_DEBUG_POSTFIX}  optimized osgParticle
    debug osgShadow${CMAKE_DEBUG_POSTFIX}    optimized osgShadow
    debug osgDB${CMAKE_DEBUG_POSTFIX}        optimized osgDB
    debug osgGA${CMAKE_DEBUG_POSTFIX}        optimized osgGA
    debug osgText${CMAKE_DEBUG_POSTFIX}      optimized osgText
    debug osgUtil${CMAKE_DEBUG_POSTFIX}      optimized osgUtil
    debug osgViewer${CMAKE_DEBUG_POSTFIX}    optimized osgViewer
    debug OpenThreads${CMAKE_DEBUG_POSTFIX}  optimized OpenThreads
)
    
# Library.
INSTALL(TARGETS osgFXEffectCompositor
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)

# Header.
INSTALL(FILES EffectCompositor
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/osgFX)
