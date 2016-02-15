//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_TYPES_H
#define TFDCF_TYPES_H

namespace DCF {
    typedef enum {
        string,
        boolean,
        int8,
        int16,
        int32,
        int64,
        float16,
        float32,
        float64,
        data,
        message
    } StorageType;

    typedef enum {
        Default = 0,
        Administration = 1 << 0,
        Garuanted = 1 << 1
    } MessageFlags;

}

#endif //TFDCF_TYPES_H
