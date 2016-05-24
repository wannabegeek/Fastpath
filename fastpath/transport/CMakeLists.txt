set(HEADER_FILES
        TCPTransport.h
        Transport.h
        URL.h
        realm_transport.h
        TransportIOEvent.h
        SHMTransport.h
        BackoffStrategy.h
        socket/Socket.h
        socket/UnixSocket.h
        socket/TCPSocketClient.h
        socket/UnixSocketClient.h
        socket/TCPSocketServer.h
        socket/UnixSocketServer.h
        socket/TCPSocket.h
        sm/InterprocessNotifier.h
        sm/InterprocessNotifierServer.h
        sm/InterprocessNotifierClient.h
        sm/shm_allocator.h
)

set(SOURCE_FILES
        socket/Socket.cpp
        socket/UnixSocket.cpp
        socket/TCPSocketClient.cpp
        socket/UnixSocketClient.cpp
        socket/TCPSocketServer.cpp
        socket/UnixSocketServer.cpp
        TCPTransport.cpp
        URL.cpp
        realm_transport.cpp
        TransportIOEvent.cpp
        SHMTransport.cpp
        socket/TCPSocket.cpp
        sm/InterprocessNotifier.cpp
        sm/InterprocessNotifierServer.cpp
        sm/InterprocessNotifierClient.cpp
        )

add_library(transport OBJECT ${SOURCE_FILES} ${HEADER_FILES})
INSTALL(FILES ${HEADER_FILES} DESTINATION ${FASTPATH_INSTALL_INCLUDE_DIR}/transport)
INSTALL(DIRECTORY socket DESTINATION ${FASTPATH_INSTALL_INCLUDE_DIR}/transport)
INSTALL(DIRECTORY sm DESTINATION ${FASTPATH_INSTALL_INCLUDE_DIR}/transport)
