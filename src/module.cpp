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



NAN_MODULE_INIT(init) {
    Nan::HandleScope scope;
    MODULE_EXPORT("kill", Nan::New<FunctionTemplate>(Kill)->GetFunction());
    MODULE_EXPORT("sleep", Nan::New<FunctionTemplate>(Sleep)->GetFunction());
    MODULE_EXPORT("clockTicks", Nan::New<FunctionTemplate>(ClockTicks)->GetFunction());
    MODULE_EXPORT("pwuid", Nan::New<FunctionTemplate>(Pwuid)->GetFunction());
}

NODE_MODULE(psutil, init)

