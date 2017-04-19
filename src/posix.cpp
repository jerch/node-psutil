/* posix.cpp
 *
 * Copyright (C) 2017 Joerg Breitbart
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */
#include "posix.h"

using namespace v8;

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


NAN_METHOD(Sleep) {
    Nan::HandleScope scope;
    if (info.Length() != 1 || !info[0]->IsNumber()) {
        return Nan::ThrowError("usage: psutil.sleep(msec)");
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(info[0]->IntegerValue()));
    info.GetReturnValue().SetUndefined();
}


NAN_METHOD(ClockTicks) {
    Nan::HandleScope scope;
    info.GetReturnValue().Set(Nan::New<Number>(sysconf(_SC_CLK_TCK)));
}


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
