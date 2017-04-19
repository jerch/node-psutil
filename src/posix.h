/* posix.h
 *
 * Copyright (C) 2017 Joerg Breitbart
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef PSUTIL_POSIX_H
#define PSUTIL_POSIX_H

#include <chrono>
#include <thread>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include "nan.h"
#include "node.h"


NAN_METHOD(Kill);
NAN_METHOD(Sleep);
NAN_METHOD(ClockTicks);
NAN_METHOD(Pwuid);


#endif // PSUTIL_POSIX_H
