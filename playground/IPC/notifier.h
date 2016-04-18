//
// Created by Tom Fewster on 05/04/2016.
//

#ifndef FASTPATH_NOTIFIER_H_H
#define FASTPATH_NOTIFIER_H_H

#include "fastpath/config.h"

#ifdef HAVE_EVENTFD
    #include "arch/linux/notifier.h"
#else
    #include "arch/mac/notifier.h"
#endif

#endif //FASTPATH_NOTIFIER_H_H
