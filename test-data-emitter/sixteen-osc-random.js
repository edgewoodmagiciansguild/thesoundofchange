var osc = require('osc'),
	_ = require('lodash'),
	config = {
		localAddress: '0.0.0.0',
		localPort: 5001,
		remoteAddress: '0.0.0.0',
		remotePort: 5000
	},
	oscPort = new osc.UDPPort(config),
	oscList = _.range(0,8);
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
		//this sucks, because all 3 random numbers will be the same
		transmit(x, Math.random(), Math.random(), Math.random());
	});
}, 60);



