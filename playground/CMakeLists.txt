INCLUDE(CheckIncludeFiles)

find_package(Threads)
#set(CMAKE_VERBOSE_MAKEFILE on)

find_library(LIBRT_LIBRARIES rt)
if (LIBRT_LIBRARIES)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LIBRT_LIBRARIES}")
endif()

INCLUDE(CheckIncludeFiles)
INCLUDE(CheckFunctionExists)

#################################

add_executable(SharedMemoryReceiver SharedMemoryReceiver.cpp)
add_executable(SharedMemorySender SharedMemorySender.cpp)
add_executable(PerformanceTest PerformanceTest.cpp performance.h)
add_executable(SubjectTest SubjectTest.cpp)
add_executable(PoolTest PoolTest.cpp)
add_executable(KeyIndexTest KeyIndexTest.cpp)
add_executable(ipc_send ipc_send.cpp IPC/InterprocessNotifier.h IPC/InterprocessNotifier.cpp IPC/InterprocessNotifierServer.cpp IPC/InterprocessNotifierServer.h IPC/InterprocessNotifierClient.cpp IPC/InterprocessNotifierClient.h IPC/arch/linux/notifier.h IPC/arch/mac/notifier.h IPC/notifier.h)
add_executable(ipc_recv ipc_recv.cpp IPC/InterprocessNotifier.h IPC/InterprocessNotifier.cpp IPC/InterprocessNotifierServer.cpp IPC/InterprocessNotifierServer.h IPC/InterprocessNotifierClient.cpp IPC/InterprocessNotifierClient.h IPC/arch/linux/notifier.h IPC/arch/mac/notifier.h IPC/notifier.h shm_allocator.h)

#add_executable(ipc_send_mac arch/mac/ipc_send_mac.cpp)
#add_executable(ipc_recv ipc_recv.cpp arch/mac/ipc_send_mac.cpp)

target_link_libraries(SharedMemoryReceiver fastpath ${CMAKE_THREAD_LIBS_INIT}) # ${LIBRT_LIBRARIES})
target_link_libraries(SharedMemorySender fastpath ${CMAKE_THREAD_LIBS_INIT}) #${LIBRT_LIBRARIES})
target_link_libraries(PerformanceTest fastpath ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(SubjectTest ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(PoolTest fastpath ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(ipc_send fastpath ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(ipc_recv fastpath ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries(ipc_send_mac TFDCF ${CMAKE_THREAD_LIBS_INIT})
#target_link_libraries(ipc_recv_mac TFDCF ${CMAKE_THREAD_LIBS_INIT})
