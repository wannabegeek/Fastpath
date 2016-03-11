set(SOURCE_FILES
        blocking_ringbuffer.h
        console_colors.h
        daemon_process.cpp
        daemon_process.h
        logger.h
        optimize.h
        semaphore_light.h
        tfoptions.h
        tfpool.h
        tfringbuffer.h
        tfspinlock.h
        )

add_library(utils OBJECT ${SOURCE_FILES})