var osc = require('osc'),
	oscPort = new osc.UDPPort({
    localAddress: '0.0.0.0',
    localPort: 5002
	}),
	lastTime = Date.now();

oscPort.on('message', function (oscMsg) {
	var newTime = Date.now(),
		delta = newTime - lastTime;
	
    console.log(delta, oscMsg);
	lastTime = newTime;
	
});

oscPort.on('error', function (error) {
	console.log('Error: ', error);
});

oscPort.open();