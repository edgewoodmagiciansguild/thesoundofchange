// Get link to main module
app = require.main.exports;

// Load required modules
var _ = require('lodash');

var logCatcher = _.bindAll({

	init: function() {
		var self = this;

		self.log = app.logger.create('logCatcher');
		self.error = app.logger.create('logCatcher', 'error');
		self.warn = app.logger.create('logCatcher', 'warn');

		self.log('init');

		app.on('noaa:data', self.catchData);
	},

	catchData: function (station, data) {
		this.log({ station: station, data: data });
	}

});

app.on('app:start', logCatcher.init);

module.exports = logCatcher;
