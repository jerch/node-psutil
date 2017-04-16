var util = require('util');
var PosixProcess = require('./_posix');
var _psutil = require('./build/Release/psutil.node');

var fs = require('fs');

var PROCFS_PATH = '/proc/';
var PROC_STATUSES = {
    'R': 'running',
    'S': 'sleeping',
    'D': 'disk-sleep',
    'T': 'stopped',
    't': 'tracing-stop',
    'Z': 'zombie',
    'X': 'dead',
    'x': 'dead',
    'K': 'wake-kill',
    'W': 'waking'
};

var system;

function SunSystem() {
    if (!(this instanceof SunSystem))
        return new SunSystem();
	this.states = _psutil.states();
}

SunSystem.prototype.bootTime = function () {
};
SunSystem.prototype.clockTicks = function () {
	return _psutil.clockTicks();
};
SunSystem.prototype.pids = function () {
    var files = fs.readdirSync(PROCFS_PATH);
    var pids = [];
    for (var i=0; i<files.length; ++i) {
        if (!isNaN(files[i]))
            pids.push(parseInt(files[i]));
    }
    return pids;
};


function SunProcess(pid) {
    if (!(this instanceof SunProcess))
        return new SunProcess(pid);
    PosixProcess.apply(this, arguments);
}
util.inherits(SunProcess, PosixProcess);


SunProcess.prototype.name = function () {
	return _psutil.psinfo(this.pid).name;
};
SunProcess.prototype.exe = function () {
    throw new Error('not implemented');
};
SunProcess.prototype.cmdline = function () {
	return _psutil.psinfo(this.pid).args.split(' ');
};
SunProcess.prototype.environ = function () {
    throw new Error('not implemented');
};
SunProcess.prototype.terminal = function () {
	return this.getTerminalMap()[_psutil.psinfo(this.pid).ttydev] || '';
};
SunProcess.prototype.cpuTimes = function () {
    throw new Error('not implemented');
};
SunProcess.prototype.cpuNum = function () {
    throw new Error('not implemented');
};
SunProcess.prototype.createTime = function () {
	return new Date(_psutil.psinfo(this.pid).start * 1000);
};
SunProcess.prototype.memoryInfo = function () {
    throw new Error('not implemented');
};
SunProcess.prototype.cwd = function () {
    throw new Error('not implemented');
};
SunProcess.prototype.status = function () {
	return system.states[_psutil.psinfo(this.pid).state];
};
SunProcess.prototype.ppid = function () {
	return _psutil.psinfo(this.pid).ppid;
};
SunProcess.prototype.username = function () {
	return _psutil.pwuid(_psutil.psinfo(this.pid).uid).name;
};

system = SunSystem();

module.exports.system = system;
module.exports.Process = SunProcess;
