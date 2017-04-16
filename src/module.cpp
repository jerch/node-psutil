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
#include <procfs.h>
#include <sys/mkdev.h>
#include <iostream>
NAN_METHOD(Psinfo) {
    Nan::HandleScope scope;
    if (info.Length() != 1 || !info[0]->IsNumber()) {
        return Nan::ThrowError("usage: psutil.psinfo(pid)");
    }

	psinfo_t psinfo;
	char *path;
	asprintf(&path, "/proc/%d/psinfo", info[0]->Uint32Value());
	if (!path)
		return;  // TODO: some kind of error handling
	int fd = open(path, O_RDONLY);
	if (fd == -1)
		return;  // TODO: some kind of error handling
	int bytes = read(fd, (void *) &psinfo, sizeof(psinfo));
	if (bytes == -1) {
		close(fd);
		return;  // TODO: some kind of error handling
    }
	if (bytes != sizeof(psinfo)) {
		close(fd);
		return;  // TODO: some kind of error handling
	}
	close(fd);
	free(path);

    Local<Object> obj = Nan::New<Object>();
	obj->Set(Nan::New<String>("ppid").ToLocalChecked(), Nan::New<Number>(psinfo.pr_ppid));
	obj->Set(Nan::New<String>("uid").ToLocalChecked(), Nan::New<Number>(psinfo.pr_uid));
	obj->Set(Nan::New<String>("ttydev").ToLocalChecked(), Nan::New<Number>(psinfo.pr_ttydev));
    obj->Set(Nan::New<String>("start").ToLocalChecked(), Nan::New<Number>(psinfo.pr_start.tv_sec));  // TODO: more digits!
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
#endif



NAN_MODULE_INIT(init) {
    Nan::HandleScope scope;
    MODULE_EXPORT("kill", Nan::New<FunctionTemplate>(Kill)->GetFunction());
    MODULE_EXPORT("sleep", Nan::New<FunctionTemplate>(Sleep)->GetFunction());
    MODULE_EXPORT("clockTicks", Nan::New<FunctionTemplate>(ClockTicks)->GetFunction());
    MODULE_EXPORT("pwuid", Nan::New<FunctionTemplate>(Pwuid)->GetFunction());

#if defined(__sun)
    MODULE_EXPORT("psinfo", Nan::New<FunctionTemplate>(Psinfo)->GetFunction());
	MODULE_EXPORT("states", Nan::New<FunctionTemplate>(States)->GetFunction());
#endif
}

NODE_MODULE(psutil, init)

