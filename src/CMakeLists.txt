project(TFDCF)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)

CHECK_INCLUDE_FILES(pthread.h HAVE_PTHREAD_H)

CHECK_FUNCTION_EXISTS(pthread_setaffinity_np HAVE_PTHREAD_SETAFFINITY_NP)
CHECK_FUNCTION_EXISTS(pthread_setname_np HAVE_PTHREAD_SETNAME_NP)
CHECK_FUNCTION_EXISTS(pthread_getname_np HAVE_PTHREAD_GETNAME_NP)

CONFIGURE_FILE(config.h.in config.h)

include_directories(${BOOST_INCLUDE_DIR} ${CMAKE_SOURCE_DIR})

set(SOURCE_FILES
        Message.cpp
        Message.h
        Encoder.h
        Field.cpp
        Field.h
        Types.h
        concurrentqueue.h
        tfpool.h
        tfspinlock.h
        Decoder.h
        Exception.h
        MessageBuffer.h
        FieldTraits.h ByteStorage.cpp ByteStorage.h MessageCodec.cpp MessageCodec.h SharedMemoryBuffer.h)

add_library(TFDCF ${SOURCE_FILES})
