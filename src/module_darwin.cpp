/* module_darwin.cpp
 *
 * Copyright (C) 2017 Joerg Breitbart
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include "module.h"
#include "posix.h"

#include <sys/user.h>
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/proc.h>

#define MSECS(t) ((unsigned long) t.tv_sec * 1000 + t.tv_usec / 1000)

using namespace v8;


NAN_METHOD(Pids) {
    Nan::HandleScope scope;

    int err;
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_ALL, 0};
    struct kinfo_proc *pinfo;
    size_t bytes;

    // repeat until we get everything into pinfo
    do {
        pinfo = NULL;
        bytes = 0;

        // first obtain bytes to hold process info
        err = sysctl(mib, 4, NULL, &bytes, NULL, 0);
        if (err == -1) {
            std::string error(strerror(errno));
            return Nan::ThrowError((std::string("pids failed - ") + error).c_str());
        }

        // obtain all process infos
        pinfo = (struct kinfo_proc *) malloc(bytes);
        if (!pinfo)
            return Nan::ThrowError("pids failed - not enough memory");
        err = sysctl(mib, 4, pinfo, &bytes, NULL, 0);
        if (err == -1) {
            if (errno == ENOMEM) {
                free(pinfo);
                pinfo = NULL;
            } else {
                std::string error(strerror(errno));
                return Nan::ThrowError((std::string("pids failed - ") + error).c_str());
            }
        }
    } while (!pinfo);

    unsigned int length = bytes / sizeof(struct kinfo_proc);

    // write PIDs to JS array
    Local<Array> arr(Nan::New<Array>(length));
    for (unsigned int i=0; i<length; ++i)
        arr->Set(i, Nan::New<Number>(pinfo[i].kp_proc.p_pid));

    free(pinfo);

    info.GetReturnValue().Set(arr);
}


NAN_METHOD(Cmdline) {
    Nan::HandleScope scope;
    if (info.Length() != 1 || !info[0]->IsNumber()) {
        return Nan::ThrowError("usage: psutil.cmdline(pid)");
    }

    int mib[] = {CTL_KERN, KERN_PROCARGS2, (int) info[0]->IntegerValue()};
    char *pinfo = NULL;
    size_t bytes = 0;
    size_t size = sizeof(size_t);

    int mib_[] = {CTL_KERN, KERN_ARGMAX};
    if (sysctl(mib_, 2, &bytes, &size, NULL, 0) == -1) {
        std::string error(strerror(errno));
        return Nan::ThrowError((std::string("cmdline failed1 - ") + error).c_str());
    }

    pinfo = (char *) malloc(bytes);
    if (!pinfo)
        return Nan::ThrowError("cmdline failed - not enough memory");
    if (sysctl(mib, 3, pinfo, &bytes, NULL, 0) == -1) {
            std::string error(strerror(errno));
            return Nan::ThrowError((std::string("cmdline2 failed - ") + error).c_str());
    }
    info.GetReturnValue().Set(Nan::CopyBuffer(pinfo, bytes).ToLocalChecked());
    free(pinfo);
}


NAN_METHOD(Kinfo) {
    Nan::HandleScope scope;
    if (info.Length() != 1 || !info[0]->IsNumber()) {
        return Nan::ThrowError("usage: psutil.kinfo(pid)");
    }

    struct kinfo_proc pinfo;
    size_t bytes = sizeof(pinfo);
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, (int) info[0]->IntegerValue()};

    if (sysctl(mib, 4, &pinfo, &bytes, NULL, 0) == -1) {
        std::string error(strerror(errno));
        return Nan::ThrowError((std::string("kinfo failed - ") + error).c_str());
    }
    Local<Object> obj = Nan::New<Object>();
    obj->Set(Nan::New<String>("ppid").ToLocalChecked(), Nan::New<Number>(pinfo.kp_eproc.e_ppid));
    obj->Set(Nan::New<String>("tdev").ToLocalChecked(), Nan::New<Number>(pinfo.kp_eproc.e_tdev));
    obj->Set(Nan::New<String>("name").ToLocalChecked(), Nan::CopyBuffer(pinfo.kp_proc.p_comm, MAXCOMLEN+1).ToLocalChecked());
    obj->Set(Nan::New<String>("start").ToLocalChecked(), Nan::New<Number>(MSECS(pinfo.kp_proc.p_un.__p_starttime)));
    obj->Set(Nan::New<String>("uid").ToLocalChecked(), Nan::New<Number>(pinfo.kp_eproc.e_pcred.p_ruid));
    obj->Set(Nan::New<String>("status").ToLocalChecked(), Nan::New<Number>(pinfo.kp_proc.p_stat));
    info.GetReturnValue().Set(obj);
}


NAN_METHOD(States) {
    Nan::HandleScope scope;
    Local<Object> obj = Nan::New<Object>();
    obj->Set(Nan::New<Number>(SIDL), Nan::New<String>("idle").ToLocalChecked());
    obj->Set(Nan::New<Number>(SRUN), Nan::New<String>("running").ToLocalChecked());
    obj->Set(Nan::New<Number>(SSLEEP), Nan::New<String>("sleeping").ToLocalChecked());
    obj->Set(Nan::New<Number>(SSTOP), Nan::New<String>("stopped").ToLocalChecked());
    obj->Set(Nan::New<Number>(SZOMB), Nan::New<String>("zombie").ToLocalChecked());
    //obj->Set(Nan::New<Number>(LSDEAD), Nan::New<String>("zombie").ToLocalChecked());
    //obj->Set(Nan::New<Number>(LSONPROC), Nan::New<String>("running").ToLocalChecked());
    //obj->Set(Nan::New<Number>(LSSUSPENDED), Nan::New<String>("suspended").ToLocalChecked());
    info.GetReturnValue().Set(obj);
}


NAN_METHOD(Boottime) {
    Nan::HandleScope scope;

    int mib[] = {CTL_KERN, KERN_BOOTTIME};
    struct timeval pinfo = timeval();
    size_t size = sizeof(struct timeval);

    if (sysctl(mib, 2, &pinfo, &size, NULL, 0) == -1) {
            std::string error(strerror(errno));
            return Nan::ThrowError((std::string("boottime failed - ") + error).c_str());
    }
    info.GetReturnValue().Set(Nan::New<Number>(MSECS(pinfo)));
}



NAN_MODULE_INIT(init) {
    Nan::HandleScope scope;
    MODULE_EXPORT("kill", Nan::New<FunctionTemplate>(Kill)->GetFunction());
    MODULE_EXPORT("sleep", Nan::New<FunctionTemplate>(Sleep)->GetFunction());
    MODULE_EXPORT("clockTicks", Nan::New<FunctionTemplate>(ClockTicks)->GetFunction());
    MODULE_EXPORT("pwuid", Nan::New<FunctionTemplate>(Pwuid)->GetFunction());

    MODULE_EXPORT("pids", Nan::New<FunctionTemplate>(Pids)->GetFunction());
    MODULE_EXPORT("cmdline", Nan::New<FunctionTemplate>(Cmdline)->GetFunction());
    MODULE_EXPORT("kinfo", Nan::New<FunctionTemplate>(Kinfo)->GetFunction());
    MODULE_EXPORT("states", Nan::New<FunctionTemplate>(States)->GetFunction());
    MODULE_EXPORT("boottime", Nan::New<FunctionTemplate>(Boottime)->GetFunction());
}

NODE_MODULE(psutil, init)
