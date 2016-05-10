
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
        peer_connection.cpp
        peer_connection.h
        )

target_link_libraries(fprouter fastpath ${CMAKE_THREAD_LIBS_INIT} ${LIBRT_LIBRARIES})

