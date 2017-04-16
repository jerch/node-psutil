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

function LinuxSystem() {
    if (!(this instanceof LinuxSystem))
        return new LinuxSystem();
    this._boot_time = null;
}

LinuxSystem.prototype.bootTime = function () {
    if (this._boot_time)
        return this._boot_time;
    var data = fs.readFileSync(PROCFS_PATH + 'stat', encoding='UTF-8').split('\n');
    for (var i=0; i<data.length; ++i) {
        if (data[i].indexOf('btime') !== -1) {
            var seconds = parseInt(data[i].split(' ')[1]);
            this._boot_time = new Date(seconds * 1000);
            return this._boot_time;
        }
    }
};
LinuxSystem.prototype.clockTicks = function () {
    return _psutil.clockTicks();
};
LinuxSystem.prototype.pids = function () {
    var files = fs.readdirSync(PROCFS_PATH);
    var pids = [];
    for (var i=0; i<files.length; ++i) {
        if (!isNaN(files[i]))
            pids.push(parseInt(files[i]));
    }
    return pids;
};


function LinuxProcess(pid) {
    if (!(this instanceof LinuxProcess))
        return new LinuxProcess(pid);
    PosixProcess.apply(this, arguments);
}
util.inherits(LinuxProcess, PosixProcess);


LinuxProcess.prototype._parseStatFile = function() {
    var data = fs.readFileSync(PROCFS_PATH + this.pid + '/stat', encoding='UTF-8');
    return data.split(' ');
};
LinuxProcess.prototype.name = function () {
    return this._parseStatFile()[1].slice(1, -1);
};
LinuxProcess.prototype.exe = function () {
    throw new Error('not implemented');
};
LinuxProcess.prototype.cmdline = function () {
    var data = fs.readFileSync(PROCFS_PATH + this.pid + '/cmdline', encoding='UTF-8');
    return data.split('\x00').slice(0, -1);
};
LinuxProcess.prototype.environ = function () {
    throw new Error('not implemented');
};
LinuxProcess.prototype.terminal = function () {
    var tty_nr = parseInt(this._parseStatFile()[6]);
    return this.getTerminalMap()[tty_nr] || '';
};
LinuxProcess.prototype.cpuTimes = function () {
    throw new Error('not implemented');
};
LinuxProcess.prototype.cpuNum = function () {
    throw new Error('not implemented');
};
LinuxProcess.prototype.createTime = function () {
    return new Date(
        parseInt(this._parseStatFile()[21])
        * 1000
        / system.clockTicks()
        + system.bootTime().getTime());
};
LinuxProcess.prototype.memoryInfo = function () {
    throw new Error('not implemented');
};
LinuxProcess.prototype.cwd = function () {
    throw new Error('not implemented');
};
LinuxProcess.prototype.status = function () {
    return PROC_STATUSES[this._parseStatFile()[2]] || '?';
};
LinuxProcess.prototype.ppid = function () {
    return parseInt(this._parseStatFile()[3]);
};
LinuxProcess.prototype.username = function () {
    return _psutil.pwuid(fs.statSync(PROCFS_PATH + this.pid).uid).name;
};

system = LinuxSystem();

module.exports.system = system;
module.exports.Process = LinuxProcess;
