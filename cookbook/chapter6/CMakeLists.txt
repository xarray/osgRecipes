# Example 1: Using the bump mapping technique
SET(EXAMPLE_NAME cookbook_06_01)
SET(EXAMPLE_FILES ch06_01/bump_mapping.cpp)
START_EXAMPLE()

# Example 2: Simulating the view-dependent shadow
CHECK_INCLUDE_FILES (osgShadow/ViewDependentShadowMap HAVE_VIEWDEPENDENTSHADOWMAP_H)
IF(HAVE_VIEWDEPENDENTSHADOWMAP_H)
  SET(EXAMPLE_NAME cookbook_06_02)
  SET(EXAMPLE_FILES ch06_02/shadow.cpp)
  SET(EXTERNAL_LIBRARIES "debug;osgShadow${CMAKE_DEBUG_POSTFIX};optimized;osgShadow")
  START_EXAMPLE()
ENDIF(HAVE_VIEWDEPENDENTSHADOWMAP_H)

# Example 3: Implementing transparency with multiple passes
SET(EXAMPLE_NAME cookbook_06_03)
SET(EXAMPLE_FILES ch06_03/multipass.cpp)
SET(EXTERNAL_LIBRARIES "debug;osgFX${CMAKE_DEBUG_POSTFIX};optimized;osgFX")
START_EXAMPLE()

# Example 4: Reading and displaying the depth buffer
SET(EXAMPLE_NAME cookbook_06_04)
SET(EXAMPLE_FILES ch06_04/read_depth.cpp)
START_EXAMPLE()

# Example 5: Implementing the night vision effect
SET(EXAMPLE_NAME cookbook_06_05)
SET(EXAMPLE_FILES ch06_05/night_vision.cpp)
START_EXAMPLE()

# Example 6: Implementing the depth-of-field effect
SET(EXAMPLE_NAME cookbook_06_06)
SET(EXAMPLE_FILES ch06_06/depth_of_field.cpp)
START_EXAMPLE()

# Example 7: Designing a skybox with the cubemap
SET(EXAMPLE_NAME cookbook_06_07)
SET(EXAMPLE_FILES ch06_07/main.cpp
                  ch06_07/SkyBox.cpp
                  ch06_07/SkyBox)
START_EXAMPLE()

# Example 8: Creating simple water effect
SET(EXAMPLE_NAME cookbook_06_08)
SET(EXAMPLE_FILES ch06_08/simple_water.cpp)
START_EXAMPLE()

# Example 9: Creating a piece of cloud
FIND_PACKAGE(OpenGL)
IF(OPENGL_FOUND AND OPENGL_GLU_FOUND)
    SET(EXAMPLE_NAME cookbook_06_09)
    SET(EXAMPLE_FILES ch06_09/main.cpp
                      ch06_09/CloudBlock.cpp
                      ch06_09/CloudBlock)
    SET(EXTERNAL_INCLUDE_DIR "${OPENGL_INCLUDE_DIR}")
    SET(EXTERNAL_LIBRARIES ${OPENGL_gl_LIBRARY} ${OPENGL_glu_LIBRARY})
    START_EXAMPLE()
ENDIF(OPENGL_FOUND AND OPENGL_GLU_FOUND)

# Example 10: Customizing the state attribute
FIND_PATH(CG_INCLUDE_PATH Cg/cg.h)
FIND_LIBRARY(CG_GL_LIBRARY CgGL)
FIND_LIBRARY(CG_LIBRARY Cg)
IF(CG_INCLUDE_PATH AND CG_LIBRARY AND CG_GL_LIBRARY)
    SET(EXAMPLE_NAME cookbook_06_10)
    SET(EXAMPLE_FILES ch06_10/main.cpp
                      ch06_10/CgProgram.cpp
                      ch06_10/CgProgram)
    SET(EXTERNAL_INCLUDE_DIR "${CG_INCLUDE_PATH}")
    SET(EXTERNAL_LIBRARIES ${CG_GL_LIBRARY} ${CG_LIBRARY})
    START_EXAMPLE()
ENDIF(CG_INCLUDE_PATH AND CG_LIBRARY AND CG_GL_LIBRARY)

# Example 11: Using MRT to create the G-buffer
SET(EXAMPLE_NAME cookbook_06_11)
SET(EXAMPLE_FILES ch06_11/g_buffer.cpp)
START_EXAMPLE()

# Example 12: Completing the deferred shading algorithm
SET(EXAMPLE_NAME cookbook_06_12)
SET(EXAMPLE_FILES ch06_12/deferred_shading.cpp)
START_EXAMPLE()
