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
	transmit = function (oscillator, pitch, cutoff, delay) {
		oscPort.send({
			address: '/osc',
			args: [oscillator, pitch, cutoff, delay]
		});
	},
	lfo = function* (rate, multiplier, offset) {
		var x = 1;
		while (true) {
			yield (multiplier * Math.sin(x)) + offset;
			x = x + (x * rate);
			if (x > 5)
				x = 1;
		}
	},
	pitch = [],
	cutoff = [],
	delay = [];

oscPort.on('message', function (oscMsg) {
    console.log('An OSC message just arrived!', oscMsg);
});
 
oscPort.open();

oscList.forEach(function (x) {
	pitch.push(lfo(.01 * x, 1, 0));
	cutoff.push(lfo(.001, .5, .25));
	delay.push(lfo(.25, 1, 0));
});


setInterval(function () {
	oscList.forEach(function (x) {
		transmit(x, pitch[x].next().value, cutoff[x].next().value, delay[x].next().value);
	});
}, 20);



