cmake_minimum_required(VERSION 2.8)
project(fastpath)

find_library(LIBRT_LIBRARIES rt)

set(GENERIC_COMPILER_FLAGS "-Wall -pedantic -std=c++14 -fPIC -mtune=native")
IF (CMAKE_COMPILER_IS_GNUCXX)
    set(GENERIC_COMPILER_FLAGS  "${GENERIC_COMPILER_FLAGS} -Wold-style-cast -Woverloaded-virtual -flto")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
# This really need the -flto, but it is currently broken in clang
    set(GENERIC_COMPILER_FLAGS  "${GENERIC_COMPILER_FLAGS} -Wc++11-extensions")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${GENERIC_COMPILER_FLAGS} -g -gdwarf-2 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${GENERIC_COMPILER_FLAGS} -Ofast -DNDEBUG")

find_package(Boost 1.60.0)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
    set(Boost_USE_STATIC_LIBS ON)
endif()

#set(BOOST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/boost/any/include/")

include_directories(${BOOST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})

add_subdirectory(fastpath)
add_subdirectory(tests)
add_subdirectory(playground)

#SET(CPACK_GENERATOR "RPM")
SET(CPACK_PACKAGE_NAME "fastpath")
SET(CPACK_PACKAGE_SUMMARY "Fast Path middleware messaging system")
SET(CPACK_PACKAGE_SUMMARY "")
SET(CPACK_PACKAGE_VERSION "0.0.1")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_PACKAGE_VENDOR "Velox Capital Group")

INCLUDE(CPack)
