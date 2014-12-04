//TODO
//gpio load spi [buffer size in KB] przed zalaczeniem spi

var bind = require('./build/Release/wirebind'),
	Epoll = require('epoll').Epoll,
	$ = require('enderscore'),
	fs = require('fs'),
	util = require("util"),
	bufferIO = new Buffer(1),
	Emitter = require('events').EventEmitter,
	gpioPath = '/sys/class/gpio/';

var exp = {
	version: '0.0.3'
};

var getRev = exp.getRev = function(cb){
	var err = new Error('RaspberryPI is overvolted!');

	function getRev(data){
		var match = data.match(/Revision\s*:\s*([0-9a-f]*)/);
		var num = parseInt(match[1], 16);

		switch (true){
			case num<4:
				rev = 'A';
				break;
			case num<16:
				rev = 'B';
				break;
			case match[1].length>4:
				rev = false;
				break;
			default:
				rev = 'B+';
		}

		return {version: rev, revision: match[1]};
	}

	if(cb) {
		fs.readFile('/proc/cpuinfo', 'utf8', function(err, data) {
			if (err)
				return cb(err);

			var ret = getRev(data);

			if(!ret.version)
				return cb(err);

			return cb(null, ret.version, ret.revision);
		});
	} else {
		var data = fs.readFileSync('/proc/cpuinfo', 'utf8');
		var ret = getRev(data);

		if(!ret.version)
			throw err;

		return ret;
	}
};

var mods = {
	INPUT: 'in',
	OUTPUT: 'out',
	PWM_OUTPUT: 2,
	GPIO_CLOCK: 3,
	LOW: new Buffer('0'),
	HIGH: new Buffer('1'),
	// Pull up/down/none
	PUD_OFF: 0,
	PUD_DOWN: 1,
	PUD_UP: 2,
	// PWM
	PWM_MODE_MS: 0,
	PWM_MODE_BAL: 1,
	// Interrupt levels
	INT_EDGE_NONE: 'none',
	INT_EDGE_SETUP: 0,
	INT_EDGE_FALLING: 'falling',
	INT_EDGE_RISING: 'rising',
	INT_EDGE_BOTH: 'both'
};

var rev = exp.rev = getRev().version;

var pins = exp.pins = {
	1: {name: '3.3V', able: false},
	2: {name: '5V', able: false},
	3: {name: ['SDA0','GPIO0', 'WPIO8'], gpio: 0, able: true},
	4: {name: '5V', able: false},
	5: {name: ['SCL0','GPIO1', 'WPIO9'], gpio: 1, able: true},
	6: {name: 'ground', able: false},
	7: {name: ['GPIO4', 'WPIO7'], gpio: 4, able: true},
	8: {name: ['TX0','GPIO14', 'WPIO15'], gpio: 14, able: true},
	9: {name: 'ground', able: false},
	10: {name: ['RX0','GPIO15', 'WPIO16'], gpio: 15, able: true},
	11: {name: ['GPIO17', 'WPIO0'], gpio: 17, able: true, pullable: true},
	12: {name: ['PWM0','GPIO18', 'WPIO1'], gpio: 18, able: true, pullable: true},
	13: {name: ['GPIO21', 'WPIO2'], gpio: 21, able: true, pullable: true},
	14: {name: 'ground', able: false},
	15: {name: ['GPIO22', 'WPIO3'], gpio: 22, able: true, pullable: true},
	16: {name: ['GPIO23', 'WPIO4'], gpio: 23, able: true, pullable: true},
	17: {name: '3.3V', able: false},
	18: {name: ['GPIO24', 'WPIO5'], gpio: 24, able: true, pullable: true},
	19: {name: ['MOSI0','GPIO10', 'WPIO12'], gpio: 10, able: true},
	20: {name: 'ground', able: false},
	21: {name: ['MISO0','GPIO9', 'WPIO13'], gpio: 9, able: true},
	22: {name: ['GPIO25', 'WPIO6'], gpio: 25, able: true, pullable: true},
	23: {name: ['SCLK0','GPIO11', 'WPIO14'], gpio: 11, able: true},
	24: {name: ['CE0','GPIO8', 'WPIO10'], gpio: 8, able: true},
	25: {name: 'ground', able: false},
	26: {name: ['CE1','GPIO7', 'WPIO11'], gpio: 7, able: true}
};

//revB
if(rev=='B') {
	$.extend(pins, {
		3: {name: ['SDA0','GPIO2','WPIO8'], gpio: 2, able: true},
		5: {name: ['SCL0','GPIO3','WPIO9'], gpio: 3, able: true},
		13: {name: ['GPIO27','WPIO2'], gpio: 27, able: true, pullable: true},
		//P5
		27: {name: '5V', able: false},
		28: {name: '3.3V', able: false},
		29: {name: ['SDA1','GPIO28','WPIO17'], gpio: 28, able: true},
		30: {name: ['SCL1','GPIO29','WPIO18'], gpio: 29, able: true},
		31: {name: ['GPIO30','WPIO19'], gpio: 30, able: true},
		32: {name: ['GPIO31','WPIO20'], gpio: 31, able: true},
		33: {name: 'ground', able: false},
		34: {name: 'ground', able: false}
	});
} else if(rev=='B+') {
	//revB+
	$.extend(pins, {
		27: {name: 'SD0', able: false},
		28: {name: 'SC0', able: false},
		29: {name: 'GPIO5', gpio: 5, able: true},
		30: {name: 'ground', able: false},
		31: {name: 'GPIO6', gpio: 6, able: true},
		32: {name: 'GPIO12', gpio: 12, able: true},
		33: {name: 'GPIO13', gpio: 13, able: true},
		34: {name: 'ground', able: false},
		35: {name: 'GPIO19', gpio: 19, able: true},
		36: {name: 'GPIO16', gpio: 16, able: true},
		37: {name: 'GPIO26', gpio: 26, able: true},
		38: {name: 'GPIO20', gpio: 20, able: true},
		39: {name: 'ground', able: false},
		40: {name: 'GPIO21', gpio: 21, able: true}
	});
}

function attachPin(gpio, self) {
	var pin = gpio;

	if($.isString(pin)) {
		var ret = -1;

		for(var num in pins) {
			if(!$.isArray(pins[num].name)) {
				if(pins[num].name===gpio){
					ret = num;
					break;
				}
			} else {
				ret = pins[num].name.indexOf(gpio);
				if(ret!==-1){
					ret = num;
					break;
				}
			}
		}
		pin = ret;
	}

	if ($.has(pins, pin) && pins[pin].able && !pins[pin].used)
		pins[pin].used = self;
	else
		throw Error('GPIO '+gpio+' is already reserved!');

	return pin;
}

function changeHandler(err, fd, event) {
	var self = this;
	this.read(function(err, value) {
		self.emit('change', self);
		if(value)
			self.emit('up', self);
		else
			self.emit('down', self, value);
	});
}

//GPIO
var Gpio = exp.Gpio = function(gpio, options, cb) {
	var self = this;

	options = options || {};

	$.defaults(options, {
		edge: mods.INT_EDGE_BOTH,
		mode: mods.INPUT,
		pull: mods.PUD_OFF,
		state: 0
	});

	$.extend(this, $.pick(options, 'mode', 'pull'));

	this.pin = attachPin(gpio, this);
	this.gpio = pins[this.pin].gpio;
	this.path = '/sys/class/gpio/gpio'+this.gpio+'/';
	this.opts = options;


	Emitter.call(this);
	this.on('newListener', function(type) {
		if(Emitter.listenerCount(self, 'change') > 0 || Emitter.listenerCount(self, 'up') > 0 || Emitter.listenerCount(self, 'down') > 0){
			fs.read(self.fd, bufferIO, 0, 1, 0, function(err){
				if(err)
					return self.emit('error', this, new Error('Problem with setting interrupt'));

				self.epoll.add(self.fd, Epoll.EPOLLPRI);
			});
		}
	});

	this.on('removeListener', function(type) {
		if (Emitter.listenerCount(self, 'change') == 0 && Emitter.listenerCount(self, 'up') == 0 && Emitter.listenerCount(self, 'down') == 0){
			self.epoll.remove(self.fd);
		}
	});

	//export
	if(!fs.existsSync(this.path))
		fs.writeFileSync(gpioPath + 'export', this.gpio);

	fs.chmodSync(this.path+'/value', 0666);

	this.setDirection(options.mode);				//ignore if output
	this.setEdge(options.edge);						//ignore if output
	this.setPull(options.pull);						//ignore if output

	this.fd = fs.openSync(this.path+'/value', 'r+');
	this.read();									//ignore if output
	this.write(options.state);						//ignore if input

	this.epoll = new Epoll(changeHandler.bind(self));
};

//Emitter
util.inherits(Gpio, Emitter);
//Gpio.prototype = new Emitter();

Gpio.prototype.read = function(cb) {
	if(cb) {
		fs.read(this.fd, bufferIO, 0, 1, 0, function(err, bytes, buf) {
			if(err)
				return cb(err);

			cb(null, $.toInteger(bufferIO));
		});
	} else {
		fs.readSync(this.fd, bufferIO, 0, 1, 0);
		return bufferIO.readUInt8(0);
	}
};

//TODO what when input?
Gpio.prototype.write = function(value, cb) {
	if(!$.isBuffer(value))
		value = $.toBoolean(value) ? mods.HIGH : mods.LOW;

	if(this.isInput()) {
		if(cb)
			return cb(null, -1);

		return -1;
	}

	if(cb)
		fs.write(this.fd, value, 0, 1, 0, cb);
	else
		return fs.writeSync(this.fd, value, 0, 1, 0);
};

Gpio.prototype.up = function(cb) {
	this.write(true, cb);
};

Gpio.prototype.down = function(cb) {
	this.write(false, cb);
};

Gpio.prototype.setEdge = function(edge) {
	if(this.isInput) {
		fs.writeFileSync(this.path+'edge', edge);
		this.edge = edge;
		return 1;
	}

	return -1;
};

//is necessary fd save?
Gpio.prototype.setDirection = function(dir) {
	//1 for output
	if(dir!==mods.INPUT && dir!==mods.OUTPUT)
		dir = $.toBoolean(dir) ? mods.OUTPUT : mods.INPUT;

	fs.writeFileSync(this.path+'direction', dir);
	this.mode = dir;
};

Gpio.prototype.direction = function() {
	return this.isInput();
};

Gpio.prototype.setPull = function(pud) {
	if(this.isInput && this.isPullable()) {
		bind._pullUpDnControlSync(this.gpio, pud);
		this.pull = pud;
		return 1;
	}

	return -1;
};

Gpio.prototype.isPullable = function() {
	return !!pins[this.pin].pullable;
};

Gpio.prototype.isInput = function() {
	return this.mode===mods.INPUT;
};

Gpio.prototype.isOutput = function() {
	return this.mode===mods.OUTPUT;
};

Gpio.prototype.options = function() {
	return this.opts;
};

Gpio.prototype.unexport = function() {
	this.removeAllListeners();
	fs.writeFileSync(gpioPath + 'unexport', this.gpio);
};

//GROUP
var Group = exp.Group = function(gpios) {
	if(!$.isArray(gpios) || !gpios.every(function(gpio){return (gpio instanceof Gpio);}))
		throw new Error('Argument must by array of Gpio');

	this.gpios = gpios;
};

Group.prototype.write = function(values, cb) {
	if(!$.isArray(values))
		values = $.toBits(values, this.gpios.length);

	if(cb) {
		var promise = {cb: cb, count: this.gpios.length};
		this.gpios.forEach(function(gpio, index){
			gpio.write(values[index], (function(err){
				if(err){
					this.cb(err);
					this.count = 0;
				}

				if(!--this.count)
					this.cb(null);
			}).bind(promise));
		});
	} else {
		this.gpios.forEach(function(gpio){
			gpio.write(values[index]);
		});
	}
};

Group.prototype.read = function(value, cb) {
	if(cb) {
		var promise = {cb: cb, count: this.gpios.length, values: []};
		this.gpios.forEach(function(gpio, index){
			gpio.read((function(num, err, value){
				if(err){
					this.cb(err);
					this.count = 0;
				}

				this.values[num] = value;

				if(!--this.count)
					this.cb(null, this.values);
			}).bind(promise, index));
		});
	} else {
		return this.gpios.map(function(gpio){
			return gpio.read();
		});
	}
};

Group.prototype.up = function(cb) {
	this.write($.fill(this.gpios.length, 1), cb);
};

Group.prototype.down = function(cb) {
	this.write($.fill(this.gpios.length, 0), cb);
};

//PWM
var PWM = exp.PWM = function(gpio, options, cb) {
	options = options || {};

	$.defaults(options, {
		clock: 2,
		range: 1024,
		mode: mods.PWM_MODE_BAL,
		duty: 0
	});

	$.extend(this, $.pick(options, 'mode', 'clock', 'range', 'duty'));

	this.pin = attachPin(gpio, this);
	this.gpio = pins[this.pin].gpio;
	this.opts = options;

	this.setClock(this.clock);
	this.setRange(this.range);
	this.setMode(this.mode);

	if(!this.isHW())
		throw new Error('Only available pin for PWM is 18');

	bind._pinModeSync(18, mods.PWM_OUTPUT);
	this.write(this.duty);
};

PWM.prototype.isHW = function() {
	return (this.gpio===18);
};

PWM.prototype.setRange = function(range) {
	bind._pwmSetRangeSync(range);
	this.range = range;
};

PWM.prototype.setClock = function(clock) {
	bind._pwmSetClockSync(clock);
	this.clock = clock;
};

PWM.prototype.setMode = function(mode) {
	bind._pwmSetModeSync(mode);
	this.mode = mode;
};

PWM.prototype.write = function(value, cb) {
	var self = this;

	if(cb)
		bind._pwmWrite(this.gpio, value, function (err) {
			if (err)
				return cb(err);

			self.duty = value;
			cb(null);
		});
	else {
		bind._pwmWriteSync(this.gpio, value);
		this.duty = value;
	}
};

PWM.prototype.options = function() {
	return this.opts;
};

PWM.prototype.unexport = function() {
	//TODO??
	return bind._pinModeSync(this.gpio, mods.INPUT);
};

//SPI
var SPI = exp.SPI = function(channel, options, cb) {
	var self = this;

	options = options || {};

	$.defaults(options, {
		channel: 0,
		clock: 1000000,
		preBuffering: null,
		postBuffering: null,
		words: 3
	});

	$.extend(this, $.pick(options, 'channel', 'clock', 'preBuffering', 'postBuffering', 'words'));

	//reserve required pins
	['MOSI0','MISO0','SCLK0'].forEach(function(gpio){attachPin(gpio, self)});
	this.pin = attachPin('CE'+this.channel, this);
	this.gpio = pins[this.pin].gpio;
	this.opts = options;

	bind._wiringPiSPISetupSync(this.channel, this.clock);
};

SPI.prototype.options = function() {
	return this.opt;
};

SPI.prototype.write = function(data, arg, cb) {
	this.transfer(data, arg, cb);
};

SPI.prototype.read = function(arg, cb) {
	return this.transfer(this.words, arg, cb);
};

SPI.prototype.transfer = function(data, arg, cb) {
	if($.isFunction(arg)){
		cb = arg;
		arg = null;
	}

	var self = this;
	var twrite = new Buffer(data);

	if(this.preBuffering!==null)
		twrite = this.preBuffering(twrite, arg);

	if(cb)
		bind._wiringPiSPIDataRW(this.channel, twrite, twrite.length, function(err, data){
			if(err)
				return cb(err);

			if(self.postBuffering!==null)
				data = self.postBuffering(data, arg);

			cb(null, data);
		});
	else{
		var data = bind._wiringPiSPIDataRWSync(this.channel, twrite, twrite.length);

		if(this.postBuffering!==null)
			data = this.postBuffering(data, arg);

		return data;
	}
};

SPI.prototype.unexport = function() {
	//TODO??
	return -1;
};

//LCD
var LCD = exp.LCD = function(gpios, options, cb) {
	var self = this;

	options = options || {};

	$.defaults(options, {
		rows: 2,
		cols: 16,
		bits: 4,
		rs: null,
		strb : null,
		fd: null,
		data: ''
	});

	$.extend(this, $.pick(options, 'rows', 'cols', 'bits'));

	if((gpios.length-2)!==this.bits) {
		var err = new Error('Number of pins are not equal with bits!');
		if(cb)
			cb(err);
		else
			throw err;
	}

	//reserve required pins
	this.pins = gpios.map(function(gpio){
		return attachPin(gpio, self);
	});
	this.gpios = this.pins.map(function(pin){
		return pins[pin].gpio;
	});
	this.opts = options;
	this.rs = this.gpios[0];
	this.strb = this.gpios[1];

	if(this.gpios.length===6)
		var args = [this.rows, this.cols, this.bits].concat(this.gpios,(this.gpios.length===6? [0,0,0,0] : []));

	this.fd = bind._lcdInitSync.apply(null, args);
};

LCD.prototype.clear = function(cb) {
	if(cb)
		bind._lcdClear(this.fd, cb);
	else
		bind._lcdClearSync(this.fd);
};

LCD.prototype.home = function(cb) {
	if(cb)
		bind._lcdHome(this.fd, cb);
	else
		bind._lcdHomeSync(this.fd);
};

LCD.prototype.write = function(buffer, cb) {
	if(cb)
		bind._lcdPuts(this.fd, buffer, cb);
	else
		bind._lcdPutsSync(this.fd, buffer);
};

LCD.prototype.setPosition = function(x, y, cb) {
	if(cb)
		bind._lcdPosition(this.fd, x, y, cb);
	else
		bind._lcdPositionSync(this.fd, x, y);
};

//export
$.extend(exp, mods);
module.exports = exp;

//setup
bind._wiringPiSetupGpioSync();