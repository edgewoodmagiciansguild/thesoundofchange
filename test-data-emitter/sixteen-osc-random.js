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
	};

oscPort.on('message', function (oscMsg) {
    console.log('An OSC message just arrived!', oscMsg);
});
 
oscPort.open();

setInterval(function () {
	oscList.forEach(function (x) {
		transmit(x.toString(16).toUpperCase(), Math.random(), Math.random(), Math.random());
	});
}, 60);



