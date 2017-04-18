var util = require('util');
var PosixProcess = require('./_posix');
var _psutil = require('./build/Release/psutil.node');

var fs = require('fs');

var system;

function FreeBSDSystem() {
    if (!(this instanceof FreeBSDSystem))
        return new FreeBSDSystem();
    this._boot_time = null;
}

FreeBSDSystem.prototype.bootTime = function () {
};
FreeBSDSystem.prototype.clockTicks = function () {
    return _psutil.clockTicks();
};
FreeBSDSystem.prototype.pids = function () {
};


function FreeBSDProcess(pid) {
    if (!(this instanceof FreeBSDProcess))
        return new FreeBSDProcess(pid);
    PosixProcess.apply(this, arguments);
}
util.inherits(FreeBSDProcess, PosixProcess);


FreeBSDProcess.prototype.name = function () {
};
FreeBSDProcess.prototype.exe = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.cmdline = function () {
};
FreeBSDProcess.prototype.environ = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.terminal = function () {
};
FreeBSDProcess.prototype.cpuTimes = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.cpuNum = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.createTime = function () {
};
FreeBSDProcess.prototype.memoryInfo = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.cwd = function () {
    throw new Error('not implemented');
};
FreeBSDProcess.prototype.status = function () {
};
FreeBSDProcess.prototype.ppid = function () {
};
FreeBSDProcess.prototype.username = function () {
};

system = FreeBSDSystem();

module.exports.system = system;
module.exports.Process = FreeBSDProcess;
