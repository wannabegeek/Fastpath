cmake_minimum_required(VERSION 2.8)
project(TFDCF)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)

CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)

CHECK_FUNCTION_EXISTS(pthread_setaffinity_np HAVE_PTHREAD_SETAFFINITY_NP)
CHECK_FUNCTION_EXISTS(pthread_setname_np HAVE_PTHREAD_SETNAME_NP)
CHECK_FUNCTION_EXISTS(pthread_getname_np HAVE_PTHREAD_GETNAME_NP)

set(GENERIC_COMPILER_FLAGS "-Wall -pedantic -std=c++14 -fPIC -mtune=native")
IF (CMAKE_COMPILER_IS_GNUCXX)
    set(GENERIC_COMPILER_FLAGS  "${GENERIC_COMPILER_FLAGS} -Wold-style-cast -Woverloaded-virtual -flto")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
# This really need the -flto, but it is currently broken in clang
    set(GENERIC_COMPILER_FLAGS  "${GENERIC_COMPILER_FLAGS} -Wc++11-extensions")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${GENERIC_COMPILER_FLAGS} -g -gdwarf-2 -DDEBUG -DNDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${GENERIC_COMPILER_FLAGS} -Ofast")

find_package(Boost 1.60.0)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

#set(BOOST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/boost/any/include/")

include_directories(${BOOST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})

add_subdirectory(fastpath)
add_subdirectory(tests)
add_subdirectory(playground)
