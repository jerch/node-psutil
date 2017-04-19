#include "nan.h"
#include "node.h"

using namespace v8;

// macro for module export
#define MODULE_EXPORT(name, symbol)                                           \
Nan::Set(target, Nan::New<String>(name).ToLocalChecked(), symbol)


NAN_METHOD(Kill) {
    Nan::HandleScope scope;
    if (info.Length() != 2
          || !info[0]->IsNumber()
          || !info[1]->IsNumber()) {
        return Nan::ThrowError("usage: psutil.kill(pid, signum)");
    }
    int pid = info[0]->IntegerValue();
    int signum = info[1]->IntegerValue();
    int err = uv_kill(pid, signum);
    if(err)
        info.GetIsolate()->ThrowException(
            node::UVException(err, "kill", uv_strerror(err), std::to_string(pid).c_str())
        );
    info.GetReturnValue().SetUndefined();
}

#include <chrono>
#include <thread>
NAN_METHOD(Sleep) {
    Nan::HandleScope scope;
    if (info.Length() != 1 || !info[0]->IsNumber()) {
        return Nan::ThrowError("usage: psutil.sleep(msec)");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(info[0]->IntegerValue()));
    info.GetReturnValue().SetUndefined();
}

#include <unistd.h>
NAN_METHOD(ClockTicks) {
    Nan::HandleScope scope;
    info.GetReturnValue().Set(Nan::New<Number>(sysconf(_SC_CLK_TCK)));
}

#include <sys/types.h>
#include <pwd.h>
NAN_METHOD(Pwuid) {
    Nan::HandleScope scope;
    if (info.Length() != 1 || !info[0]->IsNumber()) {
        return Nan::ThrowError("usage: psutil.pwuid(uid)");
    }
    struct passwd *pw = getpwuid(info[0]->Uint32Value());
    if (!pw) {
        info.GetReturnValue().Set(Nan::Null());
        return;
    }
    Local<Object> obj = Nan::New<Object>();
    obj->Set(Nan::New<String>("name").ToLocalChecked(), Nan::New<String>(pw->pw_name).ToLocalChecked());
    obj->Set(Nan::New<String>("uid").ToLocalChecked(), Nan::New<Number>(pw->pw_uid));
    obj->Set(Nan::New<String>("gid").ToLocalChecked(), Nan::New<Number>(pw->pw_gid));
    obj->Set(Nan::New<String>("gecos").ToLocalChecked(), Nan::New<String>(pw->pw_gecos).ToLocalChecked());
    obj->Set(Nan::New<String>("dir").ToLocalChecked(), Nan::New<String>(pw->pw_dir).ToLocalChecked());
    obj->Set(Nan::New<String>("shell").ToLocalChecked(), Nan::New<String>(pw->pw_shell).ToLocalChecked());
    info.GetReturnValue().Set(obj);
}

#if defined(__sun)
#define MSECS(t) (t.tv_sec * 1000 + t.tv_nsec / 1000000)

#include <procfs.h>
#include <sys/mkdev.h>
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

#include <sys/proc.h>
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

#include <utmpx.h>
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
#endif  // __sun



#if defined(__FreeBSD__)
#include <sys/user.h>
#include <sys/types.h>
#include <sys/sysctl.h>
NAN_METHOD(Pids) {
    Nan::HandleScope scope;
    
    int err;
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PROC, 0};
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
        arr->Set(i, Nan::New<Number>(pinfo[i].ki_pid));

    free(pinfo);

    info.GetReturnValue().Set(arr);
}


NAN_METHOD(Cmdline) {
    Nan::HandleScope scope;
    if (info.Length() != 1 || !info[0]->IsNumber()) {
        return Nan::ThrowError("usage: psutil.cmdline(pid)");
    }
    
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_ARGS, (int) info[0]->IntegerValue()};
    char *pinfo = NULL;
    size_t bytes = 0;

    if (sysctl(mib, 4, NULL, &bytes, NULL, 0) == -1) {
        std::string error(strerror(errno));
        return Nan::ThrowError((std::string("cmdline failed - ") + error).c_str());
    }
    pinfo = (char *) malloc(bytes);
    if (!pinfo)
        return Nan::ThrowError("cmdline failed - not enough memory");
    if (sysctl(mib, 4, pinfo, &bytes, NULL, 0) == -1) {
            std::string error(strerror(errno));
            return Nan::ThrowError((std::string("cmdline failed - ") + error).c_str());
    }
    info.GetReturnValue().Set(Nan::CopyBuffer(pinfo, bytes).ToLocalChecked());
    free(pinfo);
}


#define MSECS(t) (t.tv_sec * 1000 + t.tv_usec / 1000)
NAN_METHOD(Kinfo) {
    Nan::HandleScope scope;
    if (info.Length() != 1 || !info[0]->IsNumber()) {
        return Nan::ThrowError("usage: psutil.kinfo(pid)");
    }
    
    int mib[] = {CTL_KERN, KERN_PROC, KERN_PROC_PID, (int) info[0]->IntegerValue()};
    struct kinfo_proc *pinfo = NULL;
    size_t bytes = 0;

    if (sysctl(mib, 4, NULL, &bytes, NULL, 0) == -1) {
        std::string error(strerror(errno));
        return Nan::ThrowError((std::string("kinfo failed - ") + error).c_str());
    }
    pinfo = (struct kinfo_proc *) malloc(bytes);
    if (!pinfo)
        return Nan::ThrowError("kinfo failed - not enough memory");
    if (sysctl(mib, 4, pinfo, &bytes, NULL, 0) == -1) {
        std::string error(strerror(errno));
        return Nan::ThrowError((std::string("kinfo failed - ") + error).c_str());
    }
    Local<Object> obj = Nan::New<Object>();
    obj->Set(Nan::New<String>("ppid").ToLocalChecked(), Nan::New<Number>(pinfo->ki_ppid));
    obj->Set(Nan::New<String>("tdev").ToLocalChecked(), Nan::New<Number>(pinfo->ki_tdev));
    obj->Set(Nan::New<String>("name").ToLocalChecked(), Nan::CopyBuffer(pinfo->ki_comm, COMMLEN+1).ToLocalChecked());
    obj->Set(Nan::New<String>("tname").ToLocalChecked(), Nan::CopyBuffer(pinfo->ki_tdname, TDNAMLEN+1).ToLocalChecked());
    obj->Set(Nan::New<String>("start").ToLocalChecked(), Nan::New<Number>(MSECS(pinfo->ki_start)));
    obj->Set(Nan::New<String>("uid").ToLocalChecked(), Nan::New<Number>(pinfo->ki_ruid));
    obj->Set(Nan::New<String>("status").ToLocalChecked(), Nan::New<Number>(pinfo->ki_stat));
    info.GetReturnValue().Set(obj);
    free(pinfo);
}


#include <sys/proc.h>
NAN_METHOD(States) {
    Nan::HandleScope scope;
    Local<Object> obj = Nan::New<Object>();
    obj->Set(Nan::New<Number>(SSLEEP), Nan::New<String>("sleeping").ToLocalChecked());
    obj->Set(Nan::New<Number>(SRUN), Nan::New<String>("running").ToLocalChecked());
    obj->Set(Nan::New<Number>(SZOMB), Nan::New<String>("zombie").ToLocalChecked());
    obj->Set(Nan::New<Number>(SSTOP), Nan::New<String>("stopped").ToLocalChecked());
    obj->Set(Nan::New<Number>(SIDL), Nan::New<String>("idle").ToLocalChecked());
    obj->Set(Nan::New<Number>(SWAIT), Nan::New<String>("waiting").ToLocalChecked());
    obj->Set(Nan::New<Number>(SLOCK), Nan::New<String>("locked").ToLocalChecked());
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


#endif  // __FreeBSD__





NAN_MODULE_INIT(init) {
    Nan::HandleScope scope;
    MODULE_EXPORT("kill", Nan::New<FunctionTemplate>(Kill)->GetFunction());
    MODULE_EXPORT("sleep", Nan::New<FunctionTemplate>(Sleep)->GetFunction());
    MODULE_EXPORT("clockTicks", Nan::New<FunctionTemplate>(ClockTicks)->GetFunction());
    MODULE_EXPORT("pwuid", Nan::New<FunctionTemplate>(Pwuid)->GetFunction());

#if defined(__sun)
    MODULE_EXPORT("psinfo", Nan::New<FunctionTemplate>(Psinfo)->GetFunction());
    MODULE_EXPORT("states", Nan::New<FunctionTemplate>(States)->GetFunction());
    MODULE_EXPORT("boottime", Nan::New<FunctionTemplate>(Boottime)->GetFunction());
#endif

#if defined(__FreeBSD__)
    MODULE_EXPORT("pids", Nan::New<FunctionTemplate>(Pids)->GetFunction());
    MODULE_EXPORT("cmdline", Nan::New<FunctionTemplate>(Cmdline)->GetFunction());
    MODULE_EXPORT("kinfo", Nan::New<FunctionTemplate>(Kinfo)->GetFunction());
    MODULE_EXPORT("states", Nan::New<FunctionTemplate>(States)->GetFunction());
    MODULE_EXPORT("boottime", Nan::New<FunctionTemplate>(Boottime)->GetFunction());
#endif
}

NODE_MODULE(psutil, init)

