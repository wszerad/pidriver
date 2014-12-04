## pidriver

Node.js bind of [wiringPi](http://wiringpi.com/) lib

## Installation

    $ [sudo] npm install pidriver

## Usage

Some functions may need root privileges

	$ [sudo] node myapp

API is at most sync but any function that have callback(cb) can by async
 ```js
 //sync
 var ret = input.write(0);

 //async
 input.write(0, function(err, ret){
 	console.log(arguments);
 })
 ```

#### Using "mods"
```js
	var pi = require('pidriver');

	pi.INPUT	//'in'

	//all mods:

	var mods = {
		//Gpio mode:
		INPUT: 'in',
		OUTPUT: 'out',

		//for internal use rather (best not to use)
		PWM_OUTPUT: 2,
		GPIO_CLOCK: 3,

		//Gpio states:
		LOW: new Buffer('0'),
		HIGH: new Buffer('1'),

		// Gpio pull
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
```

### Gpio
***

```js
var pi = require('pidriver'),
    led = new pi.Gpio('GPIO27', {mode: pi.OUTPUT}),
    input = new pi.Gpio('GPIO24', {pull: pi.PUD_UP});

input.on('up', function(input){
	console.log('Button is pressed');
	led.up();	//light up led
});

input.on('down', function(input){
	console.log('Button is unpressed');
	led.down();	//turn off led
});
```

#### new Gpio(gpio [,options])

**gpio** - pin number / 'GPIOx' / 'WPIOx' / 'special name'
> You can use pin number or one of most popular names: 'GPIOx' for official gpio numbering, 'WPIOx' for wiringPi gpio numbering or its special name like 'PWM0' (for 12s pin)
> [pin map](http://wiringpi.com/pins/)

Full list of available names:
```js
var pi = require('pidriver');

console.log(pi.pins)

/*	example for 12s pin:
	{
		...
		12: {name: ['PWM0','GPIO18', 'WPIO1'], gpio: 18, able: true, pullable: true},
		...
	}

	Pin number: 12
	Available names: PWM0, GPIO18, WPIO1
	Usable as Gpio
	Pullable (see below)
*/
```

**options**

```js
	//default
	{
		edge: pi.INT_EDGE_BOTH,
		mode: pi.INPUT,
		pull: pi.PUD_OFF,
		state: 0	//initial state
	}
```

About mode and pull [wiringPi doc](http://wiringpi.com/reference/core-functions/),
use "mods" for edge, mode, pull

Gpio is also Event Emitter:

> Gpio.on('event', cb)
>
> events:
> 'change' - state change
> 'up' - state 1
> 'down' - state 0

#### Gpio methods
+ Gpio.read([cb])			- read state

+ Gpio.write(state [,cb])	- set state

+ Gpio.up([cb])			- set state high

+ Gpio.down([cb])			- set state low

+ Gpio.setEdge(edge)

+ Gpio.setDirection(dir)

+ Gpio.setPull(pud)

+ Gpio.isPullable()

+ Gpio.isInput()

+ Gpio.isOutput()

+ Gpio.direction()		- get direction

+ Gpio.options()			- get options

+ Gpio.unexport()

### Group
***
```js
	var pi = require('pidriver'),
	led0 = new pi.Gpio('GPIO27', {mode: _.OUTPUT}),
	led1 = new pi.Gpio('GPIO22', {mode: _.OUTPUT}),
	led2 = new pi.Gpio('GPIO22', {mode: _.OUTPUT});

	var group = new pi.Group([led0, led1, led2]);
	group.up();	//light up all leds
	group.write(5);	// turn on leds: led0 and led2
```

#### new Group(gpios)

**gpios** - Array of Gpio

#### Group methods
+ Group.read([cb])

+ Group.write(value [,cb])
> value is number (you can use hex(0xFF) also)
> number is formatted to bits array (85 -> 01010101) and asian to proper pin

+ Group.up([cb])

+ Group.down([cb])

### PWM
***
```js
	var pi = require('pidriver'),
	pwm = new pi.PWM('PWM0', {});

	console.log('PWM start blinking');
	var state = 0;

	setInterval(function() {
		state += 50;

		if(state>1023)
			state = 0;

		pwm.write(state);
	}, 100);
```

#### new PWM(gpio [,options])

**gpio** - gpio name, see Gpio

**options**

```js
//default
{
	clock: 2,
	range: 1024,
	mode: mods.PWM_MODE_BAL,
	duty: 0
}
```
about range, clock, mode see [PWM](http://wiringpi.com/reference/raspberry-pi-specifics/),
for mode use "mods"

#### PWM methods
+ PWM.isHW()			- is hardware PWM (only supported)

+ PWM.setRange(range)

+ PWM.setClock(clock)

+ PWM.setMode(mode)

+ PWM.write(value[, cb])

+ PWM.options()

+ PWM.unexport()

### SPI
***

	$ gpio load spi

```js
	var pi = require('pidriver'),
		spi = new pi.SPI(0, {
			preBuffering: function(buff, arg){
				buff.writeUInt8(1, 0);
				buff.writeUInt8((8+arg)<<4, 1);
				buff.writeUInt8(0, 2);
				return buff;
			},
			postBuffering: function(buff, arg){
				return ((buff[1]&3)<<8) + buff[2];
			}
		});

	//post and preBuffering code from http://www.raspberrypi-spy.co.uk/2013/10/analogue-sensors-on-the-raspberry-pi-using-an-mcp3008/
	//example of reading channel 0 on MCP3008

	setInterval(function(){
		var channel = 0;
		console.log(spi.read(channel));
	},200);
```
#### new SPI(channel [,options])
**channel** - there is only two channels (0 and 1 (CE0 and CE1 pin))

**options**
```js
	//default
	{
		channel: 0,
		clock: 1000000,
		preBuffering: null,
		postBuffering: null,
		words: 3
    }
```
#### SPI methods
+ SPI.options()

+ SPI.write(data, arg [,cb])

+ SPI.read(arg [,cb])

+ SPI.transfer(data, arg [,cb])


### LCD
***
```js
	var pi = require('pidriver'),
		lcd = new pi.LCD(['GPIO7','GPIO8','GPIO25','GPIO24','GPIO23','GPIO18']);

 	lcd.home();

 	var i = 0,
 		t = ['-','\\','|','/'];

	setInterval(function(){
		lcd.setPosition(0,1);
		lcd.write(new Buffer(t[++i%4]));
	} , 500)
```

#### new LCD(gpios [,options])
**gpios** - array of GPIO name (first is rs, second strb, channel0, channel1, ...)

**options**
```js
	//default
	{
		rows: 2,	//number of LED's rows
		cols: 16,	//number of LED's cols
		bits: 4		//number of LED's channel (4 or 8 depends on LCD and installation)
	}
```

#### LCD methods
+ LCD.clear([cb])

+ LCD.home([cb])

+ LCD.write([cb])

+ LCD.setPosition(x, y [,cb])
