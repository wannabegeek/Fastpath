find_package(GTest REQUIRED)
find_package(Threads)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)

find_library(LIBRT_LIBRARIES rt)
if (LIBRT_LIBRARIES)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LIBRT_LIBRARIES}")
endif()

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
#        SubscriptionTests.cpp
        linear_allocator_tests.cpp)

target_link_libraries(tests fastpath ${GTEST_BOTH_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

add_test(
        NAME tests
        COMMAND tests
)

