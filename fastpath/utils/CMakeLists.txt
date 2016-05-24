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
        allocate_polymorphic.h
        allocator/generic_allocator.h
        allocator/std_allocator_resource.h
        allocator/allocator_resource.h
        temp_directory.h
)

set(SOURCE_FILES
        daemon_process.cpp
)

add_library(utils OBJECT ${SOURCE_FILES} ${HEADER_FILES})
INSTALL(FILES ${HEADER_FILES} DESTINATION ${FASTPATH_INSTALL_INCLUDE_DIR}/utils)
INSTALL(DIRECTORY allocator DESTINATION ${FASTPATH_INSTALL_INCLUDE_DIR}/utils)
