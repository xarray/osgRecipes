PROJECT(OSG_Integrations)

SET(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR};${CMAKE_MODULE_PATH}")
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
INCLUDE(CheckIncludeFiles)

FIND_PACKAGE(OpenGL)
FIND_PACKAGE(GLEW)
FIND_PACKAGE(OpenCV COMPONENTS core imgproc)
FIND_PACKAGE(ALVAR)
FIND_PACKAGE(Freetype)

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

MACRO(START_LIBRARY)

    INCLUDE_DIRECTORIES(${EXTERNAL_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
    ADD_LIBRARY(${LIBRARY_NAME} MODULE ${LIBRARY_FILES})
    SET_TARGET_PROPERTIES(${LIBRARY_NAME} PROPERTIES DEBUG_POSTFIX "${CMAKE_DEBUG_POSTFIX}")
    
    TARGET_LINK_LIBRARIES(${LIBRARY_NAME}
        debug osg${CMAKE_DEBUG_POSTFIX}          optimized osg
        debug osgAnimation${CMAKE_DEBUG_POSTFIX} optimized osgAnimation
        debug osgParticle${CMAKE_DEBUG_POSTFIX}  optimized osgParticle
        debug osgDB${CMAKE_DEBUG_POSTFIX}        optimized osgDB
        debug osgGA${CMAKE_DEBUG_POSTFIX}        optimized osgGA
        debug osgText${CMAKE_DEBUG_POSTFIX}      optimized osgText
        debug osgUtil${CMAKE_DEBUG_POSTFIX}      optimized osgUtil
        debug osgViewer${CMAKE_DEBUG_POSTFIX}    optimized osgViewer
        debug OpenThreads${CMAKE_DEBUG_POSTFIX}  optimized OpenThreads
        ${EXTERNAL_LIBRARIES}
    )
    INSTALL(TARGETS ${LIBRARY_NAME} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
                                    ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
                                    LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

ENDMACRO(START_LIBRARY)

OPTION(BUILD_WITH_LITTLE_ENDIAN "Build all examples with little endian configuration (may only affect a few examples, but be careful to set)" ON)

# Wave 1
ADD_SUBDIRECTORY(osganttweakbar)      # OSG & AntTweakBar
ADD_SUBDIRECTORY(osgawesomium)        # OSG & Awesomium
ADD_SUBDIRECTORY(osgmskinect)         # OSG & Microsoft Kinect SDK
ADD_SUBDIRECTORY(osgphysx3)           # OSG & Nvidia PhysX 3.x
ADD_SUBDIRECTORY(osgspark)            # OSG & Spark Particle
ADD_SUBDIRECTORY(osgtuio)             # OSG & TUIO
ADD_SUBDIRECTORY(osgvlc)              # OSG & libVLC SDK
ADD_SUBDIRECTORY(osgdb_assimp)        # OSG & Open Asset Import Library
ADD_SUBDIRECTORY(osgdb_freeimage)     # OSG & FreeImage

# Wave 2
ADD_SUBDIRECTORY(osgbox2d)            # OSG & Box2D
ADD_SUBDIRECTORY(osgbullet)           # OSG & Bullet Physics Engine
ADD_SUBDIRECTORY(osggizmo)            # OSG & libGizmo
ADD_SUBDIRECTORY(osgmicropather)      # OSG & MicroPather
ADD_SUBDIRECTORY(osgmygui)            # OSG & MyGUI
ADD_SUBDIRECTORY(osgnikinect)         # OSG & OpenNI
ADD_SUBDIRECTORY(osgngplant)          # OSG & ngPlant
ADD_SUBDIRECTORY(osgrvo2)             # OSG & RVO2
ADD_SUBDIRECTORY(osgdb_otl)           # OSG & Oracle/ODBC/DB2 Template Library

# Wave 3
ADD_SUBDIRECTORY(osgagg)              # OSG & AGG
ADD_SUBDIRECTORY(osgalvar)            # OSG & ALVAR
ADD_SUBDIRECTORY(osgdirectwrite)      # OSG & Microsoft DirectWrite
ADD_SUBDIRECTORY(osgrecast)           # OSG & Recastnavigation
ADD_SUBDIRECTORY(osgnanovg)           # OSG & NanoVG
ADD_SUBDIRECTORY(osgswiftpp)          # OSG & Swift++
ADD_SUBDIRECTORY(osgsilverlining)     # OSG & Sundog Silverlining
ADD_SUBDIRECTORY(osgtriton)           # OSG & Sundog Triton Ocean
ADD_SUBDIRECTORY(osgdb_physfs)        # OSG & PhysicsFS

# Wave 4
ADD_SUBDIRECTORY(osgcef)              # OSG & ChromeEmbedded
ADD_SUBDIRECTORY(osgflex)             # OSG & NVIDIA Flex
ADD_SUBDIRECTORY(osgshadowworks)      # OSG & NVIDIA HBAO+

# TODO List
#ADD_SUBDIRECTORY(osgdb_cdr)           # OSG & libCDR
#ADD_SUBDIRECTORY(osggameswf)          # OSG & GameSWF
#ADD_SUBDIRECTORY(osghavokphysics)     # OSG & Havok Physics (from AnarchySDK)
#ADD_SUBDIRECTORY(osgmupdf)            # OSG & MuPDF
#ADD_SUBDIRECTORY(osgnvfx)             # OSG & NVIDIA nvFX
#ADD_SUBDIRECTORY(osgois)              # OSG & OIS
#ADD_SUBDIRECTORY(osgperceptual)       # OSG & Intel Perceptual Computing SDK
#ADD_SUBDIRECTORY(osgsmartbody)        # OSG & SmartBody
#ADD_SUBDIRECTORY(osgwoody3d)          # OSG & Woody3D
#ADD_SUBDIRECTORY(osgberkelium)        # OSG & Berkelium
#ADD_SUBDIRECTORY(osgguichan)          # OSG & GUIChan
#ADD_SUBDIRECTORY(osgcl)               # OSG & OpenCL
#ADD_SUBDIRECTORY(osgcsg)              # OSG & OpenCSG
#ADD_SUBDIRECTORY(osgcrogai)           # OSG & Crogai
#ADD_SUBDIRECTORY(osggls)              # OSG & libGLS
#ADD_SUBDIRECTORY(osgmathgl)           # OSG & MathGL
#ADD_SUBDIRECTORY(osgnewton)           # OSG & Newton Physics
#ADD_SUBDIRECTORY(osgnurbs)            # OSG & openNURBS
#ADD_SUBDIRECTORY(osgcuvid)            # OSG & NVIDIA Cuvid
#ADD_SUBDIRECTORY(osggiworks)          # OSG & NVIDIA VXGI (Global Illumination)
#ADD_SUBDIRECTORY(osgdb_nanosvg)       # OSG & NanoSVG
#ADD_SUBDIRECTORY(osganimata)          # OSG & Animata
#ADD_SUBDIRECTORY(osgdance)            # OSG & DANCE
#ADD_SUBDIRECTORY(osgmini)             # OSG & libMini
#ADD_SUBDIRECTORY(osgsteers)           # OSG & OpenSteers
#ADD_SUBDIRECTORY(osgroboop)           # OSG & ROBOOP
#ADD_SUBDIRECTORY(osgrtmidi)           # OSG & RT-Midi
#ADD_SUBDIRECTORY(osgxaos)             # OSG & XaoS
