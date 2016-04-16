set(HEADER_FILES
        DataField.h
        DateTimeField.h
        Field.h
        FieldTraits.h
        BaseMessage.h
        Message.h
        MessageField.h
        ScalarField.h
        Serializable.h
        StorageTypes.h
        SmallDataField.h
        LargeDataField.h
)

set(SOURCE_FILES
        BaseMessage.cpp
        Message.cpp
        MessageField.cpp
)

add_library(messages OBJECT ${SOURCE_FILES} ${HEADER_FILES})
INSTALL(FILES ${HEADER_FILES} DESTINATION include/fastpath/messages)
