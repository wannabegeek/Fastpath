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
        )

add_library(transport OBJECT ${SOURCE_FILES})