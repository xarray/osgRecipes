# Example 1: Setting up views on multiple screens
SET(EXAMPLE_NAME cookbook_04_01)
SET(EXAMPLE_FILES ch04_01/multiple_screen.cpp)
START_EXAMPLE()

# Example 2: Using slave cameras to simulate a power-wall
SET(EXAMPLE_NAME cookbook_04_02)
SET(EXAMPLE_FILES ch04_02/power_wall.cpp)
START_EXAMPLE()

# Example 3: Using depth partition to display huge scene
SET(EXAMPLE_NAME cookbook_04_03)
SET(EXAMPLE_FILES ch04_03/depth_partition.cpp)
START_EXAMPLE()

# Example 4: Implementing the radar map
SET(EXAMPLE_NAME cookbook_04_04)
SET(EXAMPLE_FILES ch04_04/radar_map.cpp)
START_EXAMPLE()

# Example 5: Showing the top, front and side views of a model
SET(EXAMPLE_NAME cookbook_04_05)
SET(EXAMPLE_FILES ch04_05/four_views.cpp)
START_EXAMPLE()

# Example 6: Manipulating the top, front and side views
SET(EXAMPLE_NAME cookbook_04_06)
SET(EXAMPLE_FILES ch04_06/control_four_views.cpp)
START_EXAMPLE()

# Example 7: Following a moving model
SET(EXAMPLE_NAME cookbook_04_07)
SET(EXAMPLE_FILES ch04_07/follow_model.cpp)
START_EXAMPLE()

# Example 8: Using manipulator to follow models
SET(EXAMPLE_NAME cookbook_04_08)
SET(EXAMPLE_FILES ch04_08/tracker_manipulator.cpp)
START_EXAMPLE()

# Example 9: Designing a 2D camera manipulator
SET(EXAMPLE_NAME cookbook_04_09)
SET(EXAMPLE_FILES ch04_09/main.cpp
                  ch04_09/TwoDimManipulator.cpp
                  ch04_09/TwoDimManipulator)
START_EXAMPLE()

# Example 10: Manipulating the view with joysticks
FIND_PATH(DIRECTINPUT_INCLUDE_DIR dinput.h)
FIND_LIBRARY(DIRECTINPUT_LIBRARY dinput7.lib dinput8.lib)
FIND_LIBRARY(DIRECTINPUT_GUID_LIBRARY dxguid.lib)
IF(DIRECTINPUT_INCLUDE_DIR AND DIRECTINPUT_LIBRARY)
    SET(EXAMPLE_NAME cookbook_04_10)
    SET(EXAMPLE_FILES ch04_10/main.cpp
                      ch04_10/JoystickManipulator.cpp
                      ch04_10/JoystickManipulator)
    SET(EXTERNAL_INCLUDE_DIR "${DIRECTINPUT_INCLUDE_DIR}")
    SET(EXTERNAL_LIBRARIES ${DIRECTINPUT_LIBRARY} ${DIRECTINPUT_GUID_LIBRARY})
    START_EXAMPLE()
ENDIF(DIRECTINPUT_INCLUDE_DIR AND DIRECTINPUT_LIBRARY)
