FIND_PATH(QT5_MKSPECS_DIR cmake/Qt5Core/Qt5CoreConfig.cmake
          PATHS
          $ENV{QTDIR}/mkspecs
          /usr/lib/mkspecs
          /usr/local/lib/mkspecs
)

IF(QT5_MKSPECS_DIR)
    SET(CMAKE_PREFIX_PATH
        ${CMAKE_PREFIX_PATH}
        ${QT5_MKSPECS_DIR}/cmake/Qt5Core
        ${QT5_MKSPECS_DIR}/cmake/Qt5Gui
        ${QT5_MKSPECS_DIR}/cmake/Qt5Widgets
        ${QT5_MKSPECS_DIR}/cmake/Qt5OpenGL
    )
    
    SET(QT5_ROOT_DIR ${QT5_MKSPECS_DIR}/../)
    
    INCLUDE(CheckIncludeFiles)
    FIND_PACKAGE(OpenGL)
    FIND_PACKAGE(Qt5Core)
    FIND_PACKAGE(Qt5Gui)
    FIND_PACKAGE(Qt5Widgets)
    FIND_PACKAGE(Qt5OpenGL)
    
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
    
    INCLUDE_DIRECTORIES(${OPENSCENEGRAPH_INCLUDE_DIR} ${QT5_ROOT_DIR}/include ${OPENGL_INCLUDE_DIR})
    LINK_DIRECTORIES(${OPENSCENEGRAPH_LIB_DIR} ${QT5_ROOT_DIR}/lib)
    
    ADD_DEFINITIONS(${Qt5Widgets_DEFINITIONS} ${Qt5OpenGL_DEFINITIONS})
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS}")
    
    SET(EXTERNAL_LIBRARIES QtCore5 QtGui5 QtWidgets5 QtOpenGL5 opengl32)
    #QT5_WRAP_CPP(MOC_FILES filename.h)
    
    SET(EXAMPLE_NAME osgqt5)
    SET(EXAMPLE_FILES
        osgqt5.cpp
        GraphicsWindowQt.cpp
    )
    START_EXAMPLE()
ENDIF(QT5_MKSPECS_DIR)
