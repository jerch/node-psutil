/* module.h
 *
 * Copyright (C) 2017 Joerg Breitbart
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#ifndef PSUTIL_MODULE_H
#define PSUTIL_MODULE_H

#include "nan.h"
#include "node.h"

// macro for module export
#define MODULE_EXPORT(name, symbol)                                           \
Nan::Set(target, Nan::New<String>(name).ToLocalChecked(), symbol)

#endif // PSUTIL_MODULE_H
