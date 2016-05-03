project(TFDCF)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)
INCLUDE(CheckCXXSymbolExists)

CHECK_SYMBOL_EXISTS(__GNU_LIBRARY__ "features.h" _GNU_SOURCE)

IF (_GNU_SOURCE)
    add_definitions(-D_GNU_SOURCE)
ENDIF()

CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)

find_library(LIBRT_LIBRARIES rt)                                                                                                                                                                 

if (LIBRT_LIBRARIES)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LIBRT_LIBRARIES}")
endif()

IF (DEFINED HAVE_PTHREAD_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
    check_symbol_exists(pthread_setaffinity_np pthread.h HAVE_PTHREAD_SETAFFINITY_NP)
    check_symbol_exists(pthread_setname_np pthread.h HAVE_PTHREAD_SETNAME_NP)
    check_symbol_exists(pthread_getname_np pthread.h HAVE_PTHREAD_GETNAME_NP)
    list(REMOVE_ITEM CMAKE_REQUIRED_DEFINITIONS -D_GNU_SOURCE)
ENDIF()

SET(CMAKE_REQUIRED_INCLUDES fcntl.h)
CHECK_FUNCTION_EXISTS(pipe2 HAVE_PIPE2)
SET(CMAKE_REQUIRED_INCLUDES )

CHECK_INCLUDE_FILES(sys/eventfd.h HAVE_EVENTFD_H)
IF (DEFINED HAVE_EVENTFD_H)
    check_symbol_exists(eventfd sys/eventfd.h HAVE_EVENTFD)
ENDIF()

CHECK_INCLUDE_FILES(sys/epoll.h HAVE_EPOLL_H)
IF (HAVE_EPOLL_H EQUAL 1)
    check_symbol_exists(epoll_create "sys/epoll.h" HAVE_EPOLL)
    IF (DEFINED HAVE_EPOLL)
        message(WARNING "Using epoll")
    ELSE()
        message(FATAL_ERROR "Have epoll.h but not epoll_create")
    ENDIF()
ELSE()
    message(WARNING "No epoll checking kqueue")
    CHECK_INCLUDE_FILES(sys/event.h HAVE_EVENT_H)
    IF (HAVE_EVENT_H EQUAL 1)
        check_symbol_exists(kqueue sys/event.h HAVE_KQUEUE)
    ENDIF()
ENDIF()
IF (NOT DEFINED HAVE_KQUEUE AND NOT DEFINED HAVE_EPOLL)
    message(FATAL_ERROR "Failed to determine polling method: we require either kqueue or epoll")
ENDIF()

check_cxx_symbol_exists(EPOLLET "sys/event.h" HAVE_EPOLLET)
check_cxx_symbol_exists(MSG_NOSIGNAL "sys/types.h;sys/socket.h" HAVE_NOSIGNAL)
check_cxx_symbol_exists(SO_NOSIGPIPE "sys/types.h;sys/socket.h" HAVE_NOSIGPIPE)
check_cxx_symbol_exists(IPTOS_LOWDELAY "sys/socket.h;netinet/ip.h" HAVE_IPTOS_LOWDELAY)

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

set(CMAKE_MACOSX_RPATH 1)

include_directories(${BOOST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})

set(HEADER_FILES
        Exception.h
        MessageBuffer.h
        MutableByteStorage.h
        SharedMemoryBuffer.h
        ByteStorage.h
        MessageBuffer.h
        types.h
        status.h
        config.h
        fastpath.h
)

add_library(fastpath SHARED ${HEADER_FILES} $<TARGET_OBJECTS:utils>
        $<TARGET_OBJECTS:event>
        $<TARGET_OBJECTS:messages>
        $<TARGET_OBJECTS:transport>)

add_subdirectory(utils)
add_subdirectory(event)
add_subdirectory(messages)
add_subdirectory(transport)
add_subdirectory(router)
add_subdirectory(tools)

INSTALL(TARGETS fastpath DESTINATION lib/fastpath)
INSTALL(FILES ${HEADER_FILES} DESTINATION include/fastpath)
