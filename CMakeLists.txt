PROJECT(osgRecipes)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7)
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix for Debug mode, usually d on windows")

IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

# The OpenSceneGraph 3.0 Cookbook Source code
ADD_SUBDIRECTORY(cookbook)

# The integration examples
ADD_SUBDIRECTORY(integrations)
