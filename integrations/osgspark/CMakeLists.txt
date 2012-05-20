FIND_PATH(SPARK_INCLUDE_DIR SPK.h)
FIND_LIBRARY(SPARK_LIBRARY SPARK)
FIND_LIBRARY(SPARK_GL_LIBRARY SPARK_GL)

IF(SPARK_INCLUDE_DIR AND SPARK_LIBRARY)
    SET(EXTERNAL_INCLUDE_DIR ${SPARK_INCLUDE_DIR} ${OPENGL_INCLUDE_DIR})
    SET(EXTERNAL_LIBRARIES ${SPARK_LIBRARY} ${SPARK_GL_LIBRARY} ${OPENGL_LIBRARIES})
    
    SET(EXAMPLE_NAME osgspark)
    SET(EXAMPLE_FILES osgspark.cpp
                      SparkDrawable.cpp
                      SparkUpdatingHandler.cpp
                      simple_effect.cpp
                      explosion_effect.cpp
                      fire_effect.cpp
                      rain_effect.cpp
    )
    START_EXAMPLE()
ENDIF(SPARK_INCLUDE_DIR AND SPARK_LIBRARY)
