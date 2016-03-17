//
// Created by Tom Fewster on 10/03/2016.
//

#ifndef TFDCF_OPTIMIZE_H
#define TFDCF_OPTIMIZE_H

namespace tf {
#if defined(__GNUC__)
    inline bool likely(bool x) { return __builtin_expect((x), true); }
    inline bool unlikely(bool x) { return __builtin_expect((x), false); }
#else
    inline bool likely(bool x) { return x; }
    inline bool unlikely(bool x) { return x; }
#endif
}

#endif //TFDCF_OPTIMIZE_H
