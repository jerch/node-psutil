var _psutil = require('./build/Release/psutil.node');

var EventEmitter = require('events').EventEmitter;
var util = require('util');
var glob = require('glob');
var fs = require('fs');

function PosixProcess(pid) {
    if (!(this instanceof PosixProcess))
        return new PosixProcess(pid);
    if (typeof pid === 'undefined')
        pid = process.pid;
    pid |= 0;
    EventEmitter.call(this);
    this.pid = pid;
    this.terminatePoller = null;
    this.originalTimePoller = null;
}
util.inherits(PosixProcess, EventEmitter);

PosixProcess.prototype.isRunning = function() {
    try {
        if (_psutil.kill(this.pid, 0))
            return false;
    } catch (e) {
        if (e.code !== 'EPERM')
            return false;
    }
    return true;
};

PosixProcess.prototype.on = PosixProcess.prototype.addListener = function(eventName, listener) {
    PosixProcess.super_.prototype.on.call(this, eventName, listener);
    if (eventName === 'terminate' && !this.terminatePoller && this.isRunning()) {
        this.originalTimePoller = this.createTime();
        var that = this;
        this.terminatePoller = setInterval(function () {
            if (!that.isRunning(that.pid)
                  || that.originalTimePoller.getTime() !== that.createTime().getTime()) {
                clearInterval(that.terminatePoller);
                that.terminatePoller = null;
                that.emit('terminate');
            }
        }, 50);
    }
};

PosixProcess.prototype.getTerminalMap = function () {
    var ls = glob.sync('/dev/tty*').concat(glob.sync('/dev/pts/*'));
    var map = {};
    for (var i=0; i<ls.length; ++i) {
        var stats =fs.statSync(ls[i]);
        map[stats.rdev] = ls[i];
    }
    return map;
};

PosixProcess.prototype.wait = function (callback) {
    this.on('terminate', callback);
};

PosixProcess.prototype.waitSync = function (timeout) {
    timeout = timeout || Infinity;
    var start = new Date();
    while ((new Date())-start < timeout) {
        if (!this.isRunning())
            return true;
        _psutil.sleep(50);
    }
    return false;
};


module.exports = PosixProcess;
