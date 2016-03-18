INCLUDE(CheckIncludeFiles)

find_package(Threads)
set(CMAKE_VERBOSE_MAKEFILE on)

find_library(LIBRT_LIBRARIES rt)
if (LIBRT_LIBRARIES)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LIBRT_LIBRARIES}")
endif(LIBRT_LIBRARIES)

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

add_executable(SharedMemoryReceiver SharedMemoryReceiver.cpp)
add_executable(SharedMemorySender SharedMemorySender.cpp)
add_executable(PerformanceTest PerformanceTest.cpp performance.h)
add_executable(SubjectTest SubjectTest.cpp)

target_link_libraries(SharedMemoryReceiver TFDCF ${CMAKE_THREAD_LIBS_INIT} rt)
target_link_libraries(SharedMemorySender TFDCF ${CMAKE_THREAD_LIBS_INIT} rt)
target_link_libraries(PerformanceTest TFDCF ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(SubjectTest ${CMAKE_THREAD_LIBS_INIT})

