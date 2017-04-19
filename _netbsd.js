var util = require('util');
var PosixProcess = require('./_posix');
var _psutil = require('./build/Release/psutil.node');

var fs = require('fs');

var system;

function NetBSDSystem() {
    if (!(this instanceof NetBSDSystem))
        return new NetBSDSystem();
    this._boot_time = null;
    this.states = _psutil.states();
}

NetBSDSystem.prototype.bootTime = function () {
    if (this._boot_time)
        return this._boot_time;
    this._boot_time = new Date(_psutil.boottime());
    return this._boot_time;
};
NetBSDSystem.prototype.clockTicks = function () {
    return _psutil.clockTicks();
};
NetBSDSystem.prototype.pids = function () {
    return _psutil.pids();
};


function NetBSDProcess(pid) {
    if (!(this instanceof NetBSDProcess))
        return new NetBSDProcess(pid);
    PosixProcess.apply(this, arguments);
}
util.inherits(NetBSDProcess, PosixProcess);


NetBSDProcess.prototype.name = function () {
    return _psutil.kinfo(this.pid).name.toString().split('\x00')[0];
};
NetBSDProcess.prototype.exe = function () {
    throw new Error('not implemented');
};
NetBSDProcess.prototype.cmdline = function () {
    // call will fail for kernel threads
    try {
        return _psutil.cmdline(this.pid).toString().split('\x00').slice(0, -1);
    } catch (e) {
        return [];
    }
};
NetBSDProcess.prototype.environ = function () {
    throw new Error('not implemented');
};
NetBSDProcess.prototype.terminal = function () {
    return this.getTerminalMap()[_psutil.kinfo(this.pid).tdev] || '';
};
NetBSDProcess.prototype.cpuTimes = function () {
    throw new Error('not implemented');
};
NetBSDProcess.prototype.cpuNum = function () {
    throw new Error('not implemented');
};
NetBSDProcess.prototype.createTime = function () {
    return new Date(_psutil.kinfo(this.pid).start);
};
NetBSDProcess.prototype.memoryInfo = function () {
    throw new Error('not implemented');
};
NetBSDProcess.prototype.cwd = function () {
    throw new Error('not implemented');
};
NetBSDProcess.prototype.status = function () {
	return system.states[_psutil.kinfo(this.pid).status] || '?';
};
NetBSDProcess.prototype.ppid = function () {
    return _psutil.kinfo(this.pid).ppid;
};
NetBSDProcess.prototype.username = function () {
    return _psutil.pwuid(_psutil.kinfo(this.pid).uid).name;
};

system = NetBSDSystem();

module.exports.system = system;
module.exports.Process = NetBSDProcess;
