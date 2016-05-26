
find_package(Threads)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)

set(GENERIC_COMPILER_FLAGS "-Wall -pedantic -std=c++14")
IF (CMAKE_COMPILER_IS_GNUCXX)
    set(GENERIC_COMPILER_FLAGS  "${GENERIC_COMPILER_FLAGS} -Wold-style-cast -Woverloaded-virtual")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(GENERIC_COMPILER_FLAGS  "${GENERIC_COMPILER_FLAGS} -Wc++11-extensions")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${GENERIC_COMPILER_FLAGS} -g -gdwarf-2 -DDEBUG -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${GENERIC_COMPILER_FLAGS} -Ofast")

#################################

add_executable(fprouter
        fprouter.cpp
        bootstrap.cpp
        bootstrap.h
        tcp_peer_connection.cpp
        tcp_peer_connection.h
        shm_peer_connection.cpp
        shm_peer_connection.h
        peer_connection.cpp
        peer_connection.h
        message_wrapper.cpp
        message_wrapper.h
        server_transport.h
        tcp_server_transport.h
        tcp_server_transport.cpp
        shm_server_transport.h
        shm_server_transport.cpp
        )

target_link_libraries(fprouter fastpath ${CMAKE_THREAD_LIBS_INIT} ${LIBRT_LIBRARIES})

INSTALL(TARGETS fprouter DESTINATION ${FASTPATH_INSTALL_BIN_DIR})
