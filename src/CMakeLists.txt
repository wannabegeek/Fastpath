project(TFDCF)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckCXXSymbolExists)

CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)

CHECK_FUNCTION_EXISTS(pthread_setaffinity_np HAVE_PTHREAD_SETAFFINITY_NP)
CHECK_FUNCTION_EXISTS(pthread_setname_np HAVE_PTHREAD_SETNAME_NP)
CHECK_FUNCTION_EXISTS(pthread_getname_np HAVE_PTHREAD_GETNAME_NP)
CHECK_INCLUDE_FILES (sys/epoll.h HAVE_EPOLL_H)
CHECK_INCLUDE_FILES (sys/event.h HAVE_EVENT_H)
CHECK_INCLUDE_FILES (sys/epoll.h HAVE_EPOLL_H)

CHECK_FUNCTION_EXISTS(epoll_ctl HAVE_EPOLL)
CHECK_FUNCTION_EXISTS(kevent HAVE_KEVENT)

check_cxx_symbol_exists(EPOLLET "sys/event.h" HAVE_EPOLLET)
check_cxx_symbol_exists(MSG_NOSIGNAL "sys/types.h;sys/socket.h" HAVE_NOSIGNAL)
check_cxx_symbol_exists(SO_NOSIGPIPE "sys/types.h;sys/socket.h" HAVE_NOSIGPIPE)
check_cxx_symbol_exists(IPTOS_LOWDELAY "sys/socket.h;netinet/ip.h" HAVE_IPTOS_LOWDELAY)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

include_directories(${BOOST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})

set(SOURCE_FILES
        messages/Message.cpp
        messages/Message.h
        messages/Field.h
        messages/StorageTypes.h
        utils/concurrentqueue.h
        utils/tfpool.h
        utils/tfspinlock.h
        Exception.h
        MessageBuffer.h
        messages/FieldTraits.h
        MutableByteStorage.cpp
        MutableByteStorage.h
        SharedMemoryBuffer.h
        ByteStorage.cpp
        ByteStorage.h
        MessageBuffer.h
        messages/ScalarField.h
        messages/DataField.h
        messages/MessageField.h
        messages/MessageField.cpp
        messages/Serializable.h
        messages/DateTimeField.h
        types.h
        event/Event.h
        transport/Transport.h
        event/MessageEvent.h
        event/Queue.h
        event/EventManager.h
        event/EventManager.cpp
        event/PollManager.h
        event/EventType.h
        event/PollManager_epoll.h
        event/PollManager_kqueue.h
        event/PollManager_select.h
        utils/blockingconcurrentqueue.h
        status.h
        event/TimerEvent.h
        event/TimerEvent.cpp
        event/IOEvent.h
        event/IOEvent.cpp
        event/BusySpinQueue.h
        event/BlockingQueue.h
        event/InlineQueue.h
        event/Session.h
        event/ActionNotifier.cpp
        event/ActionNotifier.h
        transport/Socket.cpp
        transport/Socket.h
        transport/SocketClient.cpp
        transport/SocketClient.h
        transport/SocketServer.cpp
        transport/SocketServer.h
        transport/TCPTransport.cpp transport/TCPTransport.h transport/URL.cpp transport/URL.h)

add_library(TFDCF ${SOURCE_FILES})
