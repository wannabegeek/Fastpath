project(TFDCF)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckCXXSymbolExists)

CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)

CHECK_FUNCTION_EXISTS(pthread_setaffinity_np HAVE_PTHREAD_SETAFFINITY_NP)
CHECK_FUNCTION_EXISTS(pthread_setname_np HAVE_PTHREAD_SETNAME_NP)
CHECK_FUNCTION_EXISTS(pthread_getname_np HAVE_PTHREAD_GETNAME_NP)
CHECK_INCLUDE_FILES (sys/epoll.h HAVE_EPOLL)
CHECK_INCLUDE_FILES (sys/event.h HAVE_EVENT)
CHECK_INCLUDE_FILES (sys/epoll.h HAVE_EPOLL)

#CHECK_FUNCTION_EXISTS(epoll_ctl HAVE_EPOLL)
CHECK_FUNCTION_EXISTS(kevent HAVE_KEVENT)

check_cxx_symbol_exists(EPOLLET "sys/event.h" HAVE_EPOLLET)
check_cxx_symbol_exists(MSG_NOSIGNAL "sys/types.h;sys/socket.h" HAVE_NOSIGNAL)
check_cxx_symbol_exists(SO_NOSIGPIPE "sys/types.h;sys/socket.h" HAVE_NOSIGPIPE)
check_cxx_symbol_exists(IPTOS_LOWDELAY "sys/socket.h;netinet/ip.h" HAVE_IPTOS_LOWDELAY)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

include_directories(${BOOST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})

set(SOURCE_FILES
        Exception.h
        MessageBuffer.h
        MutableByteStorage.cpp
        MutableByteStorage.h
        SharedMemoryBuffer.h
        ByteStorage.cpp
        ByteStorage.h
        MessageBuffer.h
        types.h
        status.h
        )

add_library(TFDCF SHARED ${SOURCE_FILES} $<TARGET_OBJECTS:utils>
        $<TARGET_OBJECTS:event>
        $<TARGET_OBJECTS:messages>
        $<TARGET_OBJECTS:transport>)

add_subdirectory(utils)
add_subdirectory(event)
add_subdirectory(messages)
add_subdirectory(transport)
add_subdirectory(router)
