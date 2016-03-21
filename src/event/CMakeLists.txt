set(SOURCE_FILES
        Event.h
        MessageListener.h
        Queue.h
        EventManager.h
        EventManager.cpp
        PollManager.h
        EventType.h
        PollManager_epoll.h
        PollManager_kqueue.h
        PollManager_select.h
        TimerEvent.h
        TimerEvent.cpp
        IOEvent.h
        IOEvent.cpp
        BusySpinQueue.h
        BlockingQueue.h
        InlineQueue.h
        Session.h
        ActionNotifier.cpp
        ActionNotifier.h
        GlobalEventManager.cpp
        GlobalEventManager.h
        InlineEventManager.cpp
        InlineEventManager.h
        Event.cpp Queue.cpp SharedQueue.h Subscriber.cpp Subscriber.h MessageListener.cpp)

add_library(event OBJECT ${SOURCE_FILES})