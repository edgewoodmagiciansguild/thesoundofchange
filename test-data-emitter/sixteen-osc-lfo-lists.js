var osc = require('osc'),
	_ = require('lodash'),
	config = {
		localAddress: '0.0.0.0',
		localPort: 5001,
		remoteAddress: '0.0.0.0',
		remotePort: 5000
	},
	oscPort = new osc.UDPPort(config),
	oscList = _.range(0, 31),
	transmit = function (oscillator, temp, precip, snowdepth) {
		oscPort.send({
			address: '/osc',
			args: [oscillator, temp, precip, snowdepth]
		});
	},
	lfo = function* (rate, multiplier, offset) {
		var x = 1;
		while (true) {
			yield ((multiplier * Math.sin(x)) + offset) | 0;
			x = x + (x * rate);
			if (x > (Math.PI + 1))
				x = 1;
		}
	},
	temp = [],
	precip = [],
	snowdepth = [];

oscPort.on('message', function (oscMsg) {
    console.log('An OSC message just arrived!', oscMsg);
});
 
oscPort.open();

oscList.forEach(function (x) {
	temp.push(lfo(.004 * x, 750, -162.5));
	precip.push(lfo(.003, 9125, 9125));
	snowdepth.push(lfo(.002, 5910, 5910));
});


setInterval(function () {
	oscList.forEach(function (x) {
		transmit(x, temp[x].next().value, precip[x].next().value, snowdepth[x].next().value);
	});
}, 20);



