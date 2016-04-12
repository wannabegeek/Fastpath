set(SOURCE_FILES
        DataField.h
        DateTimeField.h
        Field.h
        FieldTraits.h
        BaseMessage.cpp
        BaseMessage.h
        Message.cpp
        Message.h
        MessageField.cpp
        MessageField.h
        ScalarField.h
        Serializable.h
        StorageTypes.h
        SmallDataField.h LargeDataField.h)

add_library(messages OBJECT ${SOURCE_FILES})