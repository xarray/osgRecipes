SET(TUIO_SOURCE_FILES
    #TUIO/DevReceiver.cpp
    #TUIO/DevSender.cpp
    TUIO/FlashSender.cpp
    TUIO/OscReceiver.cpp
    TUIO/OscSender.cpp
    TUIO/TcpReceiver.cpp
    TUIO/TcpSender.cpp
    TUIO/TuioBlob.cpp
    TUIO/TuioClient.cpp
    TUIO/TuioContainer.cpp
    TUIO/TuioCursor.cpp
    TUIO/TuioDispatcher.cpp
    TUIO/TuioManager.cpp
    TUIO/TuioObject.cpp
    TUIO/TuioPoint.cpp
    TUIO/TuioServer.cpp
    TUIO/TuioTime.cpp
    TUIO/UdpReceiver.cpp
    TUIO/UdpSender.cpp
    
    oscpack/osc/OscOutboundPacketStream.cpp
    oscpack/osc/OscPrintReceivedElements.cpp
    oscpack/osc/OscReceivedElements.cpp
    oscpack/osc/OscTypes.cpp
    oscpack/ip/IpEndpointName.cpp
)

IF(WIN32)
    SET(TUIO_SOURCE_FILES ${TUIO_SOURCE_FILES}
        oscpack/ip/win32/NetworkingUtils.cpp
        oscpack/ip/win32/UdpSocket.cpp
    )
    SET(EXTERNAL_LIBRARIES winmm ws2_32 ${EXTERNAL_LIBRARIES})
ELSE(WIN32)
    SET(TUIO_SOURCE_FILES ${TUIO_SOURCE_FILES}
        oscpack/ip/posix/NetworkingUtils.cpp
        oscpack/ip/posix/UdpSocket.cpp
    )
ENDIF(WIN32)

IF(BUILD_WITH_LITTLE_ENDIAN)
    ADD_DEFINITIONS(-DOSC_HOST_LITTLE_ENDIAN)
ELSEIF(BUILD_WITH_LITTLE_ENDIAN)
    ADD_DEFINITIONS(-DOSC_HOST_BIG_ENDIAN)
ENDIF(BUILD_WITH_LITTLE_ENDIAN)

SET(EXTERNAL_INCLUDE_DIR oscpack TUIO)
SET(EXAMPLE_NAME osgtuio)
SET(EXAMPLE_FILES osgtuio.cpp ${TUIO_SOURCE_FILES})
START_EXAMPLE()
