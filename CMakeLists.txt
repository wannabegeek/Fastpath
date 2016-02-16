cmake_minimum_required(VERSION 3.3)
project(TFDCF)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)

CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)

CHECK_FUNCTION_EXISTS(pthread_setaffinity_np HAVE_PTHREAD_SETAFFINITY_NP)
CHECK_FUNCTION_EXISTS(pthread_setname_np HAVE_PTHREAD_SETNAME_NP)
CHECK_FUNCTION_EXISTS(pthread_getname_np HAVE_PTHREAD_GETNAME_NP)

set(GENERIC_COMPILER_FLAGS "-Wall -pedantic -std=c++14")
IF (CMAKE_COMPILER_IS_GNUCXX)
    set(GENERIC_COMPILER_FLAGS  "${GENERIC_COMPILER_FLAGS} -Wold-style-cast -Woverloaded-virtual")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(GENERIC_COMPILER_FLAGS  "${GENERIC_COMPILER_FLAGS} -Wc++11-extensions")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${GENERIC_COMPILER_FLAGS} -g -gdwarf-2 -DDEBUG -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${GENERIC_COMPILER_FLAGS} -Ofast")

set(BOOST_INCLUDE_DIR "/Users/tom/Projects/boost_1_60_0/")

CONFIGURE_FILE(src/config.h.in src/config.h)

include_directories(${BOOST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/src)

add_subdirectory(tests)

set(SOURCE_FILES
        src/Message.cpp
        src/Message.h
        src/Encoder.h
        src/Element.cpp
        src/Element.h
        src/Types.h
        src/concurrentqueue.h
        src/tfpool.h
        src/tfspinlock.h
        src/Decoder.h
        src/Exception.h
        src/MessageBuffer.cpp
        src/MessageBuffer.h src/ElementTraits.h)
add_executable(TFDCF ${SOURCE_FILES})