var util = require('util');
var PosixProcess = require('./_posix');
var _psutil = require('./build/Release/psutil.node');

var fs = require('fs');

var system;

function DarwinSystem() {
    if (!(this instanceof DarwinSystem))
        return new DarwinSystem();
    this._boot_time = null;
    this.states = _psutil.states();
}

DarwinSystem.prototype.bootTime = function () {
    if (this._boot_time)
        return this._boot_time;
    this._boot_time = new Date(_psutil.boottime());
    return this._boot_time;
};
DarwinSystem.prototype.clockTicks = function () {
    return _psutil.clockTicks();
};
DarwinSystem.prototype.pids = function () {
    return _psutil.pids();
};


function DarwinProcess(pid) {
    if (!(this instanceof DarwinProcess))
        return new DarwinProcess(pid);
    PosixProcess.apply(this, arguments);
}
util.inherits(DarwinProcess, PosixProcess);


DarwinProcess.prototype.name = function () {
    return _psutil.kinfo(this.pid).name.toString().split('\x00')[0];
};
DarwinProcess.prototype.exe = function () {
    throw new Error('not implemented');
};
DarwinProcess.prototype.cmdline = function () {
    return _psutil.cmdline(this.pid).toString();
};
DarwinProcess.prototype.environ = function () {
    throw new Error('not implemented');
};
DarwinProcess.prototype.terminal = function () {
    return this.getTerminalMap()[_psutil.kinfo(this.pid).tdev] || '';
};
DarwinProcess.prototype.cpuTimes = function () {
    throw new Error('not implemented');
};
DarwinProcess.prototype.cpuNum = function () {
    throw new Error('not implemented');
};
DarwinProcess.prototype.createTime = function () {
    return new Date(_psutil.kinfo(this.pid).start);
};
DarwinProcess.prototype.memoryInfo = function () {
    throw new Error('not implemented');
};
DarwinProcess.prototype.cwd = function () {
    throw new Error('not implemented');
};
DarwinProcess.prototype.status = function () {
	return system.states[_psutil.kinfo(this.pid).status] || '?';
};
DarwinProcess.prototype.ppid = function () {
    return _psutil.kinfo(this.pid).ppid;
};
DarwinProcess.prototype.username = function () {
    return _psutil.pwuid(_psutil.kinfo(this.pid).uid).name;
};

system = DarwinSystem();

module.exports.system = system;
module.exports.Process = DarwinProcess;
