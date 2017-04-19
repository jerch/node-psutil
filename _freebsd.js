var util = require('util');
var PosixProcess = require('./_posix');
var _psutil = require('./build/Release/psutil.node');

var fs = require('fs');

var system;

function FreeBSDSystem() {
    if (!(this instanceof FreeBSDSystem))
        return new FreeBSDSystem();
    this._boot_time = null;
    this.states = _psutil.states();
}

FreeBSDSystem.prototype.bootTime = function () {
    if (this._boot_time)
        return this._boot_time;
    this._boot_time = new Date(_psutil.boottime());
    return this._boot_time;
};
FreeBSDSystem.prototype.clockTicks = function () {
    return _psutil.clockTicks();
};
FreeBSDSystem.prototype.pids = function () {
    return _psutil.pids();
};


function FreeBSDProcess(pid) {
    if (!(this instanceof FreeBSDProcess))
        return new FreeBSDProcess(pid);
    PosixProcess.apply(this, arguments);
}
util.inherits(FreeBSDProcess, PosixProcess);


FreeBSDProcess.prototype.name = function () {
    return _psutil.kinfo(this.pid).name.toString().split('\x00')[0];
};
FreeBSDProcess.prototype.exe = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.cmdline = function () {
    return _psutil.cmdline(this.pid).toString().split('\x00').slice(0, -1);
};
FreeBSDProcess.prototype.environ = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.terminal = function () {
    return this.getTerminalMap()[_psutil.kinfo(this.pid).tdev] || '';
};
FreeBSDProcess.prototype.cpuTimes = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.cpuNum = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.createTime = function () {
    return new Date(_psutil.kinfo(this.pid).start);
};
FreeBSDProcess.prototype.memoryInfo = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.cwd = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.status = function () {
	return system.states[_psutil.kinfo(this.pid).status] || '?';
};
FreeBSDProcess.prototype.ppid = function () {
    return _psutil.kinfo(this.pid).ppid;
};
FreeBSDProcess.prototype.username = function () {
    return _psutil.pwuid(_psutil.kinfo(this.pid).uid).name;
};

system = FreeBSDSystem();

module.exports.system = system;
module.exports.Process = FreeBSDProcess;
