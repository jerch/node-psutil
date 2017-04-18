module.exports = require('./build/Release/psutil.node');
var util = require('util');


var _system;
var system;
var _Process;
switch (process.platform) {
    //case 'darwin':
    //    _system = require('./_darwin').system;
    //    _Process = require('./_darwin').Process;
    //    break;
    case 'freebsd':
        _system = require('./_freebsd').system;
        _Process = require('./_freebsd').Process;
        break;
    case 'linux':
        _system = require('./_linux').system;
        _Process = require('./_linux').Process;
        break;
    case 'sunos':
        _system = require('./_sunos').system;
        _Process = require('./_sunos').Process;
        break;
    //case 'win32':
    //    _system = require('./_win32').system;
    //    _Process = require('./_win32').Process;
    //    break;
    default:
        throw new Error('unsupported platform');
}

function System() {
    if (!(this instanceof System))
        return new System();
    this._system = _system;
}
System.prototype.bootTime = function () {
    return new Date(this._system.bootTime());
};
System.prototype.uptime = function () {
    var up = new Date().getTime() - this._system.bootTime();
    return (up / 1000) | 0;
};
System.prototype.pids = function () {
    return this._system.pids();
};
System.prototype.processes = function () {
    var pids = this._system.pids();
    var processes = {};
    for (var i=0; i<pids.length; ++i) {
            var pid = pids[i];
            processes[pid] = new Process(pid);
    }
    return processes;
};
System.prototype.tree = function (part) {
    var pids = this.pids();
    var tree = {};
    var treepart = null;
    for (var i=0; i<pids.length; ++i) {
        var branch = [];
        var pid = pids[i];
        do {
            branch.push(pid);
            pid = new Process(pid).ppid();
        } while (pid);
        var entry = tree;
        while (branch.length) {
            pid = branch.pop();
            if (!entry[pid]) {
                entry[pid] = {};
            }
            entry = entry[pid];
            if (part == pid)
                treepart = entry;
        }
    }
    return treepart || tree;
};


function Process(pid) {
    if (!(this instanceof Process))
        return new Process(pid);
    _Process.apply(this, arguments);
}
util.inherits(Process, _Process);


Process.prototype.equals = function (other) {
    return (
    this.pid === other.pid
    && this.createTime().getTime() === other.createTime().getTime());
};
Process.prototype.toJSON = function () {
    var o = {};
    o.pid = this.pid;
    o.ppid = this.ppid();
    o.name = this.name();
    o.cmdline = this.cmdline();
    o.terminal = this.terminal();
    o.status = this.status();
    o.createTime = this.createTime();
    o.username = this.username();
    return o;
};
Process.prototype.inspect = function () {
    return this.toJSON();
};
Process.prototype.toString = function () {
    return 'Process{pid: ' + this.pid + '}';
};

Process.prototype.parentProcess = function () {
    var ppid = this.ppid();
    if (ppid)
        return new Process(ppid);
    return null;
};
Process.prototype.parents = function () {
    var parents = [];
    var pid = this.pid;
    do {
        parents.push(pid);
        pid = new Process(pid).ppid();
    } while (pid);
    return parents;
};
Process.prototype.children = function () {
    var pids = system.pids();
    var children = [];
    for (var i=0; i<pids.length; ++i) {
        if (new Process(pids[i]).ppid() == this.pid)
            children.push(pids[i]);
    }
    return children;
};
Process.prototype.tree = function () {
    return system.tree(this.pid);
};


system = System();
module.exports.system = system;
module.exports.Process = Process;
