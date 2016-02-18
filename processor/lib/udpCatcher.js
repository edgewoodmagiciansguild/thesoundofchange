// Get link to main module
app = require.main.exports;

// Load required modules
var _ = require('lodash');

var udpCatcher = _.bindAll({

	init: function() {
		var self = this;

		self.log = app.logger.create('udpCatcher');
		self.error = app.logger.create('udpCatcher', 'error');
		self.warn = app.logger.create('udpCatcher', 'warn');

		self.log('init');

		app.on('noaa:data', self.catchData);
	},

	catchData: function (station, data) {
		//self.log({ station: station, data: data });
	}

});

app.on('app:start', udpCatcher.init);

module.exports = udpCatcher;
