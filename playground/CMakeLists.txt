INCLUDE(CheckIncludeFiles)

find_package(Threads)
#set(CMAKE_VERBOSE_MAKEFILE on)

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)

#################################

add_executable(PerformanceTest PerformanceTest.cpp performance.h)
add_executable(SubjectTest SubjectTest.cpp)
add_executable(AllocatorPlayground AllocatorPlayground.cpp ../fastpath/transport/sm/SharedMemoryManager.h)
add_executable(KeyIndexTest KeyIndexTest.cpp)
add_executable(ipc_send ipc_send.cpp)
add_executable(ipc_recv ipc_recv.cpp)

#add_executable(ipc_send_mac arch/mac/ipc_send_mac.cpp)
#add_executable(ipc_recv ipc_recv.cpp arch/mac/ipc_send_mac.cpp)

target_link_libraries(PerformanceTest fastpath ${CMAKE_THREAD_LIBS_INIT} ${LIBRT_LIBRARIES})
target_link_libraries(SubjectTest ${CMAKE_THREAD_LIBS_INIT} ${LIBRT_LIBRARIES})
target_link_libraries(AllocatorPlayground ${CMAKE_THREAD_LIBS_INIT} ${LIBRT_LIBRARIES})
target_link_libraries(ipc_send fastpath fastpath ${CMAKE_THREAD_LIBS_INIT} ${LIBRT_LIBRARIES})
target_link_libraries(ipc_recv fastpath fastpath ${CMAKE_THREAD_LIBS_INIT} ${LIBRT_LIBRARIES})
#target_link_libraries(ipc_send_mac TFDCF ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries(ipc_recv_mac TFDCF ${CMAKE_THREAD_LIBS_INIT})
