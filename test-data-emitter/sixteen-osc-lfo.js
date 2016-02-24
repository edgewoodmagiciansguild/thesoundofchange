var osc = require('osc'),
	_ = require('lodash'),
	config = {
		localAddress: '0.0.0.0',
		localPort: 5001,
		remoteAddress: '0.0.0.0',
		remotePort: 5000
	},
	oscPort = new osc.UDPPort(config),
	oscList = _.range(0, 15),
	transmit = function (oscillator, pitch, cutoff, delay) {
		var address = '/osc' + oscillator + '/';
		oscPort.send({
			address: address + 'pitch',
			args: pitch
		});
		oscPort.send({
			address: address + 'cutoff',
			args: pitch
		});
		oscPort.send({
			address: address + 'delay',
			args: pitch
		});
	},
	lfo = function* (rate) {
		var x = 1;
		while (true) {
			yield Math.sin(x);
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
	pitch.push(lfo(.01));
	cutoff.push(lfo(.05));
	delay.push(lfo(.25));
});


setInterval(function () {
	oscList.forEach(function (x) {
		transmit(x.toString(16).toUpperCase(), pitch[x].next().value, cutoff[x].next().value, delay[x].next().value);
	});
}, 20);



