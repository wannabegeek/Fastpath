
find_package(Threads)

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

add_executable(fpsend fpsend.cpp)
target_link_libraries(fpsend fastpath ${CMAKE_THREAD_LIBS_INIT})

add_executable(fplisten fplisten.cpp)
target_link_libraries(fplisten fastpath ${CMAKE_THREAD_LIBS_INIT})

add_executable(fpperf_source performance_source.cpp)
target_link_libraries(fpperf_source fastpath ${CMAKE_THREAD_LIBS_INIT})

add_executable(fpperf_sink performance_sink.cpp)
target_link_libraries(fpperf_sink fastpath ${CMAKE_THREAD_LIBS_INIT})
