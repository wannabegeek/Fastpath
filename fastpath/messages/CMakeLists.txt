set(HEADER_FILES
        DataField.h
        DateTimeField.h
        Field.h
        FieldTraits.h
        BaseMessage.h
        Message.h
        MutableMessage.h
        MessageField.h
        ScalarField.h
        Serializable.h
        StorageTypes.h
        SmallDataField.h
        LargeDataField.h
        subject.h
        subscription.h
        FieldAllocator.h)

set(SOURCE_FILES
        BaseMessage.cpp
        Message.cpp
        MessageField.cpp
        MutableMessage.cpp
        MessageCodec.h)

add_library(messages OBJECT ${SOURCE_FILES} ${HEADER_FILES})
INSTALL(FILES ${HEADER_FILES} DESTINATION ${FASTPATH_INSTALL_INCLUDE_DIR}/messages)
