find_package(GTest REQUIRED)
find_package(Threads)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)

set(GENERIC_COMPILER_FLAGS "-Wall -pedantic -std=c++14")
IF (CMAKE_COMPILER_IS_GNUCXX)
    set(GENERIC_COMPILER_FLAGS  "${GENERIC_COMPILER_FLAGS} -Wold-style-cast -Woverloaded-virtual")
ELSEIF(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(GENERIC_COMPILER_FLAGS  "${GENERIC_COMPILER_FLAGS} -Wc++11-extensions -D__extern_always_inline='extern __always_inline'")
ENDIF (CMAKE_COMPILER_IS_GNUCXX)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${GENERIC_COMPILER_FLAGS} -g -gdwarf-2 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${GENERIC_COMPILER_FLAGS} -Ofast -ffast-math -DNDEBUG")

include_directories(${GTEST_INCLUDE_DIRS})

#################################

add_executable(tests
        MessageTests.cpp
        FieldTests.cpp
        MessageBufferTests.cpp
        ByteStorageTests.cpp
        MutableByteStorageTests.cpp
        EventTests.cpp
        SessionTests.cpp
        QueueTests.cpp
        SocketTests.cpp
        URLTests.cpp
        TCPTransportTests.cpp
        SubscriptionTests.cpp
        )

target_link_libraries(tests TFDCF ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_test(
        NAME tests
        COMMAND tests
)

