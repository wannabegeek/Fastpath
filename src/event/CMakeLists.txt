SET(HEADER_FILES
        Event.h
        MessageListener.h
        Queue.h
        EventManager.h
        PollManager.h
        EventType.h
        arch/linux/PollManager.h
        arch/mac/PollManager.h
        PollManager_select.h
        TimerEvent.h
        IOEvent.h
        BusySpinQueue.h
        BlockingQueue.h
        InlineQueue.h
        Session.h
        ActionNotifier.h
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
        ActionNotifier.cpp
        GlobalEventManager.cpp
        InlineEventManager.cpp
        )

add_library(event OBJECT ${SOURCE_FILES} ${HEADER_FILES})
INSTALL(FILES ${HEADER_FILES} DESTINATION include/fastpath/event)
