cmake_minimum_required(VERSION 2.8)
project(fastpath)
set(PROJECT_VERSION 0.0.1)

find_library(LIBRT_LIBRARIES rt)
IF (NOT LIBRT_LIBRARIES)
    SET(LIBRT_LIBRARIES "")
ENDIF()

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

set(FASTPATH_INSTALL_INCLUDE_DIR include/fastpath/fastpath)
set(FASTPATH_INSTALL_LIB_DIR lib/fastpath)
set(FASTPATH_INSTALL_BIN_DIR bin/fastpath)

#set(BOOST_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/boost/any/include/")

include_directories(${BOOST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})

add_subdirectory(fastpath)
add_subdirectory(tests)
add_subdirectory(playground)

SET(PKG_CONFIG_LIBDIR "\${prefix}/lib/fastpath/")
SET(PKG_CONFIG_INCLUDEDIR "\${prefix}/include/fastpath/")
SET(PKG_CONFIG_LIBS "-L\${libdir} -lfastpath")
SET(PKG_CONFIG_CFLAGS "-I\${includedir}")

CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/pkg-config.pc.in ${CMAKE_CURRENT_SOURCE_DIR}/lib${PROJECT_NAME}.pc)

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/lib${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

#SET(CPACK_GENERATOR "RPM")
SET(CPACK_PACKAGE_NAME "fastpath")
SET(CPACK_PACKAGE_SUMMARY "Fast Path middleware messaging system")
SET(CPACK_PACKAGE_SUMMARY "")
SET(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "1")
SET(CPACK_PACKAGE_VENDOR "Velox Capital Group")

INCLUDE(CPack)
