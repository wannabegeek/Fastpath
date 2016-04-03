set(SOURCE_FILES
        Socket.cpp
        Socket.h
        UnixSocket.cpp
        UnixSocket.h
        TCPSocketClient.cpp
        TCPSocketClient.h
        UnixSocketClient.cpp
        UnixSocketClient.h
        TCPSocketServer.cpp
        TCPSocketServer.h
        UnixSocketServer.cpp
        UnixSocketServer.h
        TCPTransport.cpp
        TCPTransport.h
        Transport.h
        URL.cpp
        URL.h
        realm_transport.cpp
        realm_transport.h
        TransportIOEvent.cpp
        TransportIOEvent.h
        SHMTransport.cpp
        SHMTransport.h
        TCPSocket.cpp TCPSocket.h)

add_library(transport OBJECT ${SOURCE_FILES})