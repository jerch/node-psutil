/* module_sunos.cpp
 *
 * Copyright (C) 2017 Joerg Breitbart
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include "module.h"
#include "posix.h"

#include <procfs.h>
#include <sys/mkdev.h>
#include <sys/proc.h>
#include <utmpx.h>

#define MSECS(t) (t.tv_sec * 1000 + t.tv_nsec / 1000000)

using namespace v8;


NAN_METHOD(Psinfo) {
    Nan::HandleScope scope;
    if (info.Length() != 1 || !info[0]->IsNumber()) {
        return Nan::ThrowError("usage: psutil.psinfo(pid)");
    }

    psinfo_t psinfo;
    char *path;
    asprintf(&path, "/proc/%d/psinfo", info[0]->Uint32Value());
    if (!path)
        return Nan::ThrowError("psinfo failed - could not allocate path");

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        std::string error(strerror(errno));
        free(path);
        return Nan::ThrowError((std::string("psinfo failed - ") + error).c_str());
    }

    int bytes = read(fd, (void *) &psinfo, sizeof(psinfo));
    if (bytes == -1) {
        std::string error(strerror(errno));
        free(path);
        close(fd);
        return Nan::ThrowError((std::string("psinfo failed - ") + error).c_str());
    }
    if (bytes != sizeof(psinfo)) {
        free(path);
        close(fd);
        return Nan::ThrowError("psinfo failed - wrong psinfo size");
    }
    close(fd);
    free(path);

    Local<Object> obj = Nan::New<Object>();
    obj->Set(Nan::New<String>("ppid").ToLocalChecked(), Nan::New<Number>(psinfo.pr_ppid));
    obj->Set(Nan::New<String>("uid").ToLocalChecked(), Nan::New<Number>(psinfo.pr_uid));
    obj->Set(Nan::New<String>("ttydev").ToLocalChecked(), Nan::New<Number>(psinfo.pr_ttydev));
    obj->Set(Nan::New<String>("start").ToLocalChecked(), Nan::New<Number>(MSECS(psinfo.pr_start)));
    obj->Set(Nan::New<String>("name").ToLocalChecked(), Nan::New<String>(psinfo.pr_fname).ToLocalChecked());
    obj->Set(Nan::New<String>("args").ToLocalChecked(), Nan::New<String>(psinfo.pr_psargs).ToLocalChecked());
    obj->Set(Nan::New<String>("state").ToLocalChecked(), Nan::New<Number>(psinfo.pr_lwp.pr_state));
    info.GetReturnValue().Set(obj);
}


NAN_METHOD(States) {
    Nan::HandleScope scope;
    Local<Object> obj = Nan::New<Object>();
    obj->Set(Nan::New<Number>(SSLEEP), Nan::New<String>("sleeping").ToLocalChecked());
    obj->Set(Nan::New<Number>(SRUN), Nan::New<String>("running").ToLocalChecked());
    obj->Set(Nan::New<Number>(SZOMB), Nan::New<String>("zombie").ToLocalChecked());
    obj->Set(Nan::New<Number>(SSTOP), Nan::New<String>("stopped").ToLocalChecked());
    obj->Set(Nan::New<Number>(SIDL), Nan::New<String>("idle").ToLocalChecked());
    obj->Set(Nan::New<Number>(SONPROC), Nan::New<String>("running").ToLocalChecked());
    obj->Set(Nan::New<Number>(SWAIT), Nan::New<String>("waiting").ToLocalChecked());
    info.GetReturnValue().Set(obj);
}


NAN_METHOD(Boottime) {
    Nan::HandleScope scope;
    struct utmpx search;
    search.ut_type = BOOT_TIME;
    struct utmpx *ut;
    long boottime = 0;
    ut = getutxid(&search);
    if (!ut) {
        std::string error(strerror(errno));
        return Nan::ThrowError((std::string("boottime failed - ") + error).c_str());
    }
    boottime = ut->ut_tv.tv_sec;
    endutent();
    info.GetReturnValue().Set(Nan::New<Number>(boottime));
}



NAN_MODULE_INIT(init) {
    Nan::HandleScope scope;
    MODULE_EXPORT("kill", Nan::New<FunctionTemplate>(Kill)->GetFunction());
    MODULE_EXPORT("sleep", Nan::New<FunctionTemplate>(Sleep)->GetFunction());
    MODULE_EXPORT("clockTicks", Nan::New<FunctionTemplate>(ClockTicks)->GetFunction());
    MODULE_EXPORT("pwuid", Nan::New<FunctionTemplate>(Pwuid)->GetFunction());

    MODULE_EXPORT("psinfo", Nan::New<FunctionTemplate>(Psinfo)->GetFunction());
    MODULE_EXPORT("states", Nan::New<FunctionTemplate>(States)->GetFunction());
    MODULE_EXPORT("boottime", Nan::New<FunctionTemplate>(Boottime)->GetFunction());
}

NODE_MODULE(psutil, init)
