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
        rwlock.h
        tfmultiringbuffer.h
        fast_linear_allocator.h
        short_alloc.h
        )

add_library(utils OBJECT ${SOURCE_FILES})