# Example 1: Using smart and observer pointers
SET(EXAMPLE_NAME cookbook_02_01)
SET(EXAMPLE_FILES ch02_01/observer_ptr.cpp)
START_EXAMPLE()

# Example 2: Sharing and cloning objects
SET(EXAMPLE_NAME cookbook_02_02)
SET(EXAMPLE_FILES ch02_02/clone_objects.cpp)
START_EXAMPLE()

# Example 3: Computing the world bounding box of any node
SET(EXAMPLE_NAME cookbook_02_03)
SET(EXAMPLE_FILES ch02_03/bounding_box.cpp)
START_EXAMPLE()

# Example 4: Creating a running car
SET(EXAMPLE_NAME cookbook_02_04)
SET(EXAMPLE_FILES ch02_04/running_car.cpp)
START_EXAMPLE()

# Example 5: Mirroring the scene graph
SET(EXAMPLE_NAME cookbook_02_05)
SET(EXAMPLE_FILES ch02_05/mirror_scene.cpp)
START_EXAMPLE()

# Example 6: Designing a breadth-first node visitor
SET(EXAMPLE_NAME cookbook_02_06)
SET(EXAMPLE_FILES ch02_06/main.cpp
                  ch02_06/BFSVisitor.cpp
                  ch02_06/BFSVisitor)
START_EXAMPLE()

# Example 7: Implementing a background image node
SET(EXAMPLE_NAME cookbook_02_07)
SET(EXAMPLE_FILES ch02_07/background_image.cpp)
START_EXAMPLE()

# Example 8: Making your node always face to screen
SET(EXAMPLE_NAME cookbook_02_08)
SET(EXAMPLE_FILES ch02_08/billboard_node.cpp)
START_EXAMPLE()

# Example 9: Using draw callbacks to execute NVIDIA Cg functions
FIND_PATH(CG_INCLUDE_PATH Cg/cg.h)
FIND_LIBRARY(CG_GL_LIBRARY CgGL)
FIND_LIBRARY(CG_LIBRARY Cg)
IF(CG_INCLUDE_PATH AND CG_LIBRARY AND CG_GL_LIBRARY)
    SET(EXAMPLE_NAME cookbook_02_09)
    SET(EXAMPLE_FILES ch02_09/main.cpp
                      ch02_09/CgCallbacks.cpp
                      ch02_09/CgCallbacks)
    SET(EXTERNAL_INCLUDE_DIR "${CG_INCLUDE_PATH}")
    SET(EXTERNAL_LIBRARIES ${CG_GL_LIBRARY} ${CG_LIBRARY})
    START_EXAMPLE()
ENDIF(CG_INCLUDE_PATH AND CG_LIBRARY AND CG_GL_LIBRARY)

# Example 10: Implementing a compass node
SET(EXAMPLE_NAME cookbook_02_10)
SET(EXAMPLE_FILES ch02_10/main.cpp
                  ch02_10/Compass.cpp
                  ch02_10/Compass)
START_EXAMPLE()
