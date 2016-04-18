set(HEADER_FILES
        blocking_ringbuffer.h
        console_colors.h
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
        stringhash.h
        )

set(SOURCE_FILES
        daemon_process.cpp
        )

add_library(utils OBJECT ${SOURCE_FILES} ${HEADER_FILES})
INSTALL(FILES ${HEADER_FILES} DESTINATION include/fastpath/utils)
