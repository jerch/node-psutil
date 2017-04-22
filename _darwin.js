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
    try {
        var data = _psutil.cmdline(this.pid);
        var argc = new Uint32Array(data)[0];
        var s = data.toString();
        for (var i=4; i<s.length; ++i) {
            if (s[i] == '\x00')
                continue;
            break;
        }
        var basename = '';
        for (i; i<s.length; ++i) {
            if (s[i] == '\x00')
                break;
            basename += s[i];
        }
        for (i; i<s.length; ++i) {
            if (s[i] == '\x00')
                continue;
            break;
        }
        var cmdline = [];
        var part = '';
        for (i; i<s.length; ++i) {
            if (cmdline.length == argc)
                break;
            if (s[i] == '=')
                break;
            if (s[i] == '\x00') {
                if (!part.length)
                    break;
                cmdline.push(part);
                part = '';
            } else
                part += s[i];
        }
        //return [s, basename, cmdline];
        return cmdline;
    } catch (e) {
        // TODO: make it less blocking for multiple calls
        var execFile = require('child_process').execFile;
        var output = '';
        child = execFile('/bin/ps', [this.pid], function(error, stdout, stderr) {
            output = stdout;
        });
        while (!output)
            _psutil.sleep(1);
        var s = output.split('\n')[1].split(':').slice(-1)[0];
        if (!s)
            return [];
        return s.substring(s.indexOf(' ')+1).split(' ');
    }
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
