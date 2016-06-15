SET(HEADER_FILES
        Event.h
        MessageListener.h
        Queue.h
        EventManager.h
        PollManager.h
        EventType.h
        PollManager_select.h
        TimerEvent.h
        IOEvent.h
        SignalEvent.h
        BusySpinQueue.h
        BlockingQueue.h
        InlineQueue.h
        Session.h
        notifier.h
        arch/linux/notifier.h
        arch/mac/notifier.h
        GlobalEventManager.h
        InlineEventManager.h
        SharedQueue.h
        Subscriber.h
        DataEvent.h
        MessageEvent.h
        )

SET(SOURCE_FILES
        Subscriber.cpp
        MessageListener.cpp
        DataEvent.cpp
        MessageEvent.cpp
        Queue.cpp
        EventManager.cpp
        TimerEvent.cpp
        IOEvent.cpp
        SignalEvent.cpp
        GlobalEventManager.cpp
        InlineEventManager.cpp
        Session.cpp
        )

IF (HAVE_EPOLL EQUAL 1)
    message(WARNING "epoll found")
    SET(SOURCE_FILES ${SOURCE_FILES} arch/linux/PollManager.cpp)
    INSTALL(FILES arch/linux/notifier.h DESTINATION ${FASTPATH_INSTALL_INCLUDE_DIR}/event/arch/linux)
ELSEIF(HAVE_KQUEUE EQUAL 1)
    message(WARNING "kqueue found")
    SET(SOURCE_FILES ${SOURCE_FILES} arch/mac/PollManager.cpp)
    INSTALL(FILES arch/mac/notifier.h DESTINATION ${FASTPATH_INSTALL_INCLUDE_DIR}/event/arch/mac)
ELSE ()
    message(FATAL_ERROR "kqueue or epoll not found")
ENDIF()

add_library(event OBJECT ${SOURCE_FILES} ${HEADER_FILES})
INSTALL(FILES ${HEADER_FILES} DESTINATION ${FASTPATH_INSTALL_INCLUDE_DIR}/event)
