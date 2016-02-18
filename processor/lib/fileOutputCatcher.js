// Get link to main module
app = require.main.exports;

// Load required modules
var _ = require('lodash');

var fileOutputCatcher = _.bindAll({

	init: function() {
		var self = this;

		self.log = app.logger.create('fileOutputCatcher');
		self.error = app.logger.create('fileOutputCatcher', 'error');
		self.warn = app.logger.create('fileOutputCatcher', 'warn');

		self.log('init');

		app.on('noaa:data', self.catchData);
	},

	catchData: function (station, data) {
		//self.log({ station: station, data: data });
	}

});

app.on('app:start', fileOutputCatcher.init);

module.exports = fileOutputCatcher;
