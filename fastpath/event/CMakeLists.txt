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
        )

IF (HAVE_EPOLL EQUAL 1)
    message(WARNING "epoll found")
    SET(SOURCE_FILES ${SOURCE_FILES} arch/linux/PollManager.cpp)
ELSEIF(HAVE_KQUEUE EQUAL 1)
    message(WARNING "kqueue found")
    SET(SOURCE_FILES ${SOURCE_FILES} arch/mac/PollManager.cpp)
ELSE ()
    message(FATAL_ERROR "kqueue or epoll not found")
ENDIF()

add_library(event OBJECT ${SOURCE_FILES} ${HEADER_FILES})
INSTALL(FILES ${HEADER_FILES} DESTINATION include/fastpath/event)
