# Example 1: Creating a polygon with borderlines
SET(EXAMPLE_NAME cookbook_03_01)
SET(EXAMPLE_FILES ch03_01/draw_polygon.cpp)
START_EXAMPLE()

# Example 2: Extruding a 2D shape to 3D
SET(EXAMPLE_NAME cookbook_03_02)
SET(EXAMPLE_FILES ch03_02/extrusion.cpp)
START_EXAMPLE()

# Example 3: Drawing a NURBS surface
FIND_PACKAGE(OpenGL)
IF(OPENGL_FOUND AND OPENGL_GLU_FOUND)
    SET(EXAMPLE_NAME cookbook_03_03)
    SET(EXAMPLE_FILES ch03_03/main.cpp
                      ch03_03/NurbsSurface.cpp
                      ch03_03/NurbsSurface)
    SET(EXTERNAL_INCLUDE_DIR "${OPENGL_INCLUDE_DIR}")
    SET(EXTERNAL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
    START_EXAMPLE()
ENDIF(OPENGL_FOUND AND OPENGL_GLU_FOUND)

# Example 4: Drawing a dynamic clock on the screen
SET(EXAMPLE_NAME cookbook_03_04)
SET(EXAMPLE_FILES ch03_04/clock_face.cpp)
START_EXAMPLE()

# Example 5: Drawing a ribbon following a model
SET(EXAMPLE_NAME cookbook_03_05)
SET(EXAMPLE_FILES ch03_05/dynamic_ribbon.cpp)
START_EXAMPLE()

# Example 6: Selecting and highlighting a model
SET(EXAMPLE_NAME cookbook_03_06)
SET(EXAMPLE_FILES ch03_06/select_model.cpp)
START_EXAMPLE()

# Example 7: Selecting a triangle face of the model
SET(EXAMPLE_NAME cookbook_03_07)
SET(EXAMPLE_FILES ch03_07/select_face.cpp)
START_EXAMPLE()

# Example 8: Selecting a point of the model
SET(EXAMPLE_NAME cookbook_03_08)
SET(EXAMPLE_FILES ch03_08/select_point.cpp)
START_EXAMPLE()

# Example 9: Using vertex displacement mapping in shaders
SET(EXAMPLE_NAME cookbook_03_09)
SET(EXAMPLE_FILES ch03_09/vertex_displacement.cpp)
START_EXAMPLE()

# Example 10: Using the draw instanced extension
SET(EXAMPLE_NAME cookbook_03_10)
SET(EXAMPLE_FILES ch03_10/draw_instanced.cpp)
START_EXAMPLE()
