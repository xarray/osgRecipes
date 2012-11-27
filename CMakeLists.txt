PROJECT(osgRecipes)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.7)
SET(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix for Debug mode, usually d on windows")

IF(COMMAND CMAKE_POLICY)
    CMAKE_POLICY(SET CMP0003 NEW)
ENDIF(COMMAND CMAKE_POLICY)

# The OpenSceneGraph 3.0 Cookbook Source code
OPTION(BUILD_COOKBOOK "Build Cookbook Examples" ON)
IF(BUILD_COOKBOOK)
	ADD_SUBDIRECTORY(cookbook)
ENDIF(BUILD_COOKBOOK)

# The integration examples
OPTION(BUILD_INTEGRATIONS "Build integrations of OSG and other APIs" ON)
IF(BUILD_INTEGRATIONS)
    ADD_SUBDIRECTORY(integrations)
ENDIF(BUILD_INTEGRATIONS)

OPTION(BUILD_TESTS "Build experimental components" ON)
IF(BUILD_TESTS)
    # The shader effect compositor in progress
    ADD_SUBDIRECTORY(effectcompositor)
ENDIF(BUILD_TESTS)
