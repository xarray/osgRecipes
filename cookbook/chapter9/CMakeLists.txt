# Example 1: Integrating OSG with Qt
FIND_PACKAGE(Qt)
IF(QT4_FOUND)
    SET(EXAMPLE_NAME cookbook_09_01)
    SET(EXAMPLE_FILES ch09_01/osg_qt.cpp)
    SET(EXTERNAL_INCLUDE_DIR "${QT_INCLUDE_DIR}")
    SET(EXTERNAL_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
                           "debug;osgQt${CMAKE_DEBUG_POSTFIX};optimized;osgQt")
    START_EXAMPLE()
ENDIF(QT4_FOUND)

# Example 2: Starting rendering loop in separate threads
IF(QT4_FOUND)
    SET(EXAMPLE_NAME cookbook_09_02)
    SET(EXAMPLE_FILES ch09_02/osg_qthread.cpp)
    SET(EXTERNAL_INCLUDE_DIR "${QT_INCLUDE_DIR}")
    SET(EXTERNAL_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
                           "debug;osgQt${CMAKE_DEBUG_POSTFIX};optimized;osgQt")
    START_EXAMPLE()
ENDIF(QT4_FOUND)

# Example 3: Embedding Qt widgets into the scene
IF(QT4_FOUND)
    SET(EXAMPLE_NAME cookbook_09_03)
    SET(EXAMPLE_FILES ch09_03/osg_qtwidgets.cpp)
    SET(EXTERNAL_INCLUDE_DIR "${QT_INCLUDE_DIR}")
    SET(EXTERNAL_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY}
                           "debug;osgQt${CMAKE_DEBUG_POSTFIX};optimized;osgQt")
    START_EXAMPLE()
ENDIF(QT4_FOUND)

# Example 4: Embedding CEGUI elements into the scene
FIND_PACKAGE(OpenGL)
FIND_PATH(CEGUI_INCLUDE_DIR CEGUI.h)
FIND_LIBRARY(CEGUI_GL_LIBRARY CEGUIOpenGLRenderer)
FIND_LIBRARY(CEGUI_LIBRARY CEGUIBase)
IF(CEGUI_INCLUDE_DIR AND CEGUI_LIBRARY AND CEGUI_GL_LIBRARY)
    SET(EXAMPLE_NAME cookbook_09_04)
    SET(EXAMPLE_FILES ch09_04/main.cpp
                      ch09_04/CEGUIDrawable.cpp
                      ch09_04/CEGUIDrawable)
    SET(EXTERNAL_INCLUDE_DIR "${CEGUI_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR}")
    SET(EXTERNAL_LIBRARIES ${CEGUI_GL_LIBRARY} ${CEGUI_LIBRARY} ${OPENGL_gl_LIBRARY})
    START_EXAMPLE()
ENDIF(CEGUI_INCLUDE_DIR AND CEGUI_LIBRARY AND CEGUI_GL_LIBRARY)

# Example 5: Using the osgWidget library
SET(EXAMPLE_NAME cookbook_09_05)
SET(EXAMPLE_FILES ch09_05/osg_widgets.cpp)
SET(EXTERNAL_LIBRARIES "debug;osgWidget${CMAKE_DEBUG_POSTFIX};optimized;osgWidget")
START_EXAMPLE()

# Example 6: Using OSG components in GLUT
FIND_PACKAGE(GLUT)
IF(GLUT_FOUND)
    SET(EXAMPLE_NAME cookbook_09_06)
    SET(EXAMPLE_FILES ch09_06/osg_in_glut.cpp)
    SET(EXTERNAL_INCLUDE_DIR "${GLUT_INCLUDE_DIR}")
    SET(EXTERNAL_LIBRARIES ${GLUT_glut_LIBRARY})
    START_EXAMPLE()
ENDIF(GLUT_FOUND)

# Example 7: Run OSG examples on Android

# Example 8: Embedding OSG into web browsers

# Example 9: Designing the command buffer mechanism
