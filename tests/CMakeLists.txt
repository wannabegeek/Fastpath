set(GTEST_SRC_DIR googletest)
set(GTEST_LIBS gtest gtest_main)

include_directories(${GTEST_SRC_DIR}/include ${GTEST_SRC_DIR})

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

add_subdirectory(${GTEST_SRC_DIR})

#################################

add_executable(tests
        MessageTests.cpp
        FieldTests.cpp
        MessageBufferTests.cpp ByteStorageTests.cpp MutableByteStorageTests.cpp EventTests.cpp SessionTests.cpp)

target_link_libraries(tests TFDCF ${GTEST_LIBS} ${CMAKE_THREAD_LIBS_INIT})

add_test(
        NAME tests
        COMMAND tests
)

