//
// Created by Tom Fewster on 05/04/2016.
//

#ifndef TFDCF_NOTIFIER_H_H
#define TFDCF_NOTIFIER_H_H

#include "../../src/config.h"

#ifdef HAVE_EVENTFD
    #include "arch/linux/notifier.h"
#else
    #include "arch/mac/notifier.h"
#endif

#endif //TFDCF_NOTIFIER_H_H
