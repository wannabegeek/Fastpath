set(SOURCE_FILES
        Socket.cpp
        Socket.h
        SocketClient.cpp
        SocketClient.h
        SocketServer.cpp
        SocketServer.h
        TCPTransport.cpp
        TCPTransport.h
        Transport.h
        URL.cpp
        URL.h
        realm_transport.cpp realm_transport.h TransportIOEvent.cpp TransportIOEvent.h SHMTransport.cpp SHMTransport.h)

add_library(transport OBJECT ${SOURCE_FILES})