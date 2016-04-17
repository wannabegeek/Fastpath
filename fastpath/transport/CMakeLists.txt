set(HEADER_FILES
        Socket.h
        UnixSocket.h
        TCPSocketClient.h
        UnixSocketClient.h
        TCPSocketServer.h
        UnixSocketServer.h
        TCPTransport.h
        Transport.h
        URL.h
        realm_transport.h
        TransportIOEvent.h
        SHMTransport.h
        TCPSocket.h
)

set(SOURCE_FILES
        Socket.cpp
        UnixSocket.cpp
        TCPSocketClient.cpp
        UnixSocketClient.cpp
        TCPSocketServer.cpp
        UnixSocketServer.cpp
        TCPTransport.cpp
        URL.cpp
        realm_transport.cpp
        TransportIOEvent.cpp
        SHMTransport.cpp
        TCPSocket.cpp
)

add_library(transport OBJECT ${SOURCE_FILES} ${HEADER_FILES})
INSTALL(FILES ${HEADER_FILES} DESTINATION include/fastpath/transport)
