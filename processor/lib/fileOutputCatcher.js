// Get link to main module
app = require.main.exports;

// Load required modules
var _ = require('lodash'),
	fs = require('fs'),
	path = require('path');

var fileOutputCatcher = _.bindAll({

	channels: {},

	init: function() {
		var self = this;

		self.log = app.logger.create('fileOutputCatcher');
		self.error = app.logger.create('fileOutputCatcher', 'error');
		self.warn = app.logger.create('fileOutputCatcher', 'warn');

		self.log('init');

		app.on('noaa:data', self.catchData);
		app.on('noaa:done', self.sortData);
		app.on('noaa:paused', self.sortData);
	},

	catchData: function (station, data) {
		if (station !== undefined) {
			if (this.channels[station.id] === undefined) {
				this.channels[station.id] = [];
			}

			data.station = station;
			this.channels[station.id].push(data);
		}

		if (Object.keys(this.channels).length >= 35) {
			app.emit('noaa:pause');
		}
	},

	sortData: function () {
		var self = this;

		if (Object.keys(this.channels).length > 0) {
			var months = {},
				i = 0;

			self.log('sorting and saving');

			for (var i in self.channels) {
				var channel = self.channels[i];
					data = null;

				while (data !== undefined) {
					data = channel.shift();
					if (data) {
						if (months[data.id] === undefined) {
							months[data.id] = [];
						}
						months[data.id].push(data);
					}
				}

				delete self.channels[i];
			}

			self.channels = [];

			Object.keys(months).sort().forEach(function(key) {
				var days = {},
					dayKey = null,
					station = null;

				for (i in months[key]) {
					if (Array.isArray(months[key])) {
						for (var t in months[key][i]) {
							if (Array.isArray(months[key][i][t])) {
								for (var d in months[key][i][t]) {
									dayKey = key + '-' + (d.length < 2  && d !== 9 ? '0' : '') + (parseInt(d) + 1);
									station = months[key][i].station;

									if (!Number.isNaN(months[key][i][t][d].value)) {
										if (days[dayKey] === undefined) {
											days[dayKey] = {};
										}

										if (days[dayKey][station.id] === undefined) {
											days[dayKey][station.id] = {};
											//days[dayKey][station.id].day = dayKey;
										}

										if (days[dayKey][station.id][t] === undefined) {
											days[dayKey][station.id][t] = {};
										}

										days[dayKey][station.id][t] = months[key][i][t][d];
									}
								}
							}
						}
					}
				}

				self.saveData(key, days);

				delete days;
			});

			delete months;

			app.emit('noaa:resume');
		}
	},

	saveData: function (month, days) {
		var outputFilename = path.dirname(require.main.filename) + '/data/' + month + '.json';
		if (fs.existsSync(outputFilename)) {
			var obj = JSON.parse(fs.readFileSync(outputFilename, 'utf8'));
			for (var d in days) {
				if (obj[d] === undefined) {
					obj[d] = {};
				}

				for (var s in days[d]) {
					obj[d][s] = days[d][s];
				}
			}

			days = {};
			Object.keys(obj).sort().forEach(function(key) {
				days[key] = obj[key]
			});
		}

		fs.writeFileSync(outputFilename, JSON.stringify(days));
	}

});

app.on('app:start', fileOutputCatcher.init);

module.exports = fileOutputCatcher;
