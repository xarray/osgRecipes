# Example 1: Opening and closing the door
SET(EXAMPLE_NAME cookbook_05_01)
SET(EXAMPLE_FILES ch05_01/door_animation.cpp)
START_EXAMPLE()

# Example 2: Playing a movie in the 3D world
SET(EXAMPLE_NAME cookbook_05_02)
SET(EXAMPLE_FILES ch05_02/movie_texture.cpp)
START_EXAMPLE()

# Example 3: Designing scrolling texts
SET(EXAMPLE_NAME cookbook_05_03)
SET(EXAMPLE_FILES ch05_03/scroll_texts.cpp)
START_EXAMPLE()

# Example 4: Implementing a morph geometry
SET(EXAMPLE_NAME cookbook_05_04)
SET(EXAMPLE_FILES ch05_04/morph_geometry.cpp)
START_EXAMPLE()

# Example 5: Fading in and out
SET(EXAMPLE_NAME cookbook_05_05)
SET(EXAMPLE_FILES ch05_05/fade_inout.cpp)
START_EXAMPLE()

# Example 6: Animating a flight on fire
SET(EXAMPLE_NAME cookbook_05_06)
SET(EXAMPLE_FILES ch05_06/cessna_fire.cpp)
SET(EXTERNAL_LIBRARIES "debug;osgParticle${CMAKE_DEBUG_POSTFIX};optimized;osgParticle")
START_EXAMPLE()

# Example 7: Dynamically lighting within shaders
SET(EXAMPLE_NAME cookbook_05_07)
SET(EXAMPLE_FILES ch05_07/dynamic_light.cpp)
START_EXAMPLE()

# Example 8: Creating a simple Galaxian game
SET(EXAMPLE_NAME cookbook_05_08)
SET(EXAMPLE_FILES ch05_08/main.cpp
                  ch05_08/Player.cpp
                  ch05_08/Player)
START_EXAMPLE()

# Example 9: Building a skeleton system
SET(EXAMPLE_NAME cookbook_05_09)
SET(EXAMPLE_FILES ch05_09/simple_skeleton.cpp)
START_EXAMPLE()

# Example 10: Skinning a skeleton system
SET(EXAMPLE_NAME cookbook_05_10)
SET(EXAMPLE_FILES ch05_10/bind_skin.cpp)
START_EXAMPLE()

# Example 11: Letting the physics engine be
FIND_PATH(PHYSX_SDK_DIR Physics/include/NxPhysics.h)
FIND_LIBRARY(PHYSX_LIBRARY PhysXLoader.lib libPhysXLoader.so)
IF(PHYSX_INCLUDE_DIR AND PHYSX_LIBRARY)
    SET(EXAMPLE_NAME cookbook_05_11)
    SET(EXAMPLE_FILES ch05_11/main.cpp
                      ch05_11/PhysXInterface.cpp
                      ch05_11/PhysXInterface)
    SET(EXTERNAL_INCLUDE_DIR
        "${PHYSX_SDK_DIR}/PhysXLoader/include"
        "${PHYSX_SDK_DIR}/Physics/include"
        "${PHYSX_SDK_DIR}/Foundation/include")
    SET(EXTERNAL_LIBRARIES ${PHYSX_LIBRARY})
    START_EXAMPLE()
ENDIF(PHYSX_INCLUDE_DIR AND PHYSX_LIBRARY)
