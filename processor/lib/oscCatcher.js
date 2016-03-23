// Get link to main module
app = require.main.exports;

// Load required modules
var _ = require('lodash'),
	osc = require('osc');

var oscCatcher = _.bindAll({

	oscChannels: {},
	playStack: [],
	channels: app.config.oscCatcher.channels,
	playInterval: null,
	oscPort: null,
	delay: app.config.oscCatcher.delay,

	lastYear: null,

	init: function() {
		var self = this;

		self.log = app.logger.create('oscCatcher');
		self.error = app.logger.create('oscCatcher', 'error');
		self.warn = app.logger.create('oscCatcher', 'warn');

		self.log('init');

		var config = {
			localAddress: '0.0.0.0',
			localPort: 5001,
			remoteAddress: app.config.udpEmitter.host,
			remotePort: app.config.udpEmitter.port
		};

		self.oscPort = new osc.UDPPort(config);
		self.oscPort.open();

		app.on('noaa:data', self.catchData);
		app.on('noaa:done', self.playData);
	},

	catchData: function (station, data) {
		if (station !== undefined) {
			if (Object.keys(this.oscChannels).length < this.channels) {
				if (this.oscChannels[station.id] === undefined) {
					this.oscChannels[station.id] = [];
				}
			}

			if (this.oscChannels[station.id] !== undefined) {
				data.station = station;
				this.oscChannels[station.id].push(data);
			}
		}
	},

	playData: function () {
		var self = this;

		self.log('playing');

		var months = {},
			i = 0;

		for (i in self.oscChannels) {
			var data = null;
			while (data !== undefined) {
				data = self.oscChannels[i].shift();
				if (data) {
					if (months[data.id] === undefined) {
						months[data.id] = [];
					}
					months[data.id].push(data);
				}
			}
		}

		Object.keys(months).sort().forEach(function(key) {
			var days = {},
				dayKey = null,
				station = null;

			for (i in months[key]) {
				if (Array.isArray(months[key])) {
					for (var t in months[key][i]) {
						if (Array.isArray(months[key][i][t])) {
							for (var d in months[key][i][t]) {
								dayKey = key + '-' + (parseInt(d) + 1);
								station = months[key][i].station;

								if (!Number.isNaN(months[key][i][t][d].value)) {
									if (days[dayKey] === undefined) {
										days[dayKey] = {};
									}

									if (days[dayKey][station.id] === undefined) {
										days[dayKey][station.id] = {};
										days[dayKey][station.id].day = dayKey;
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

			//console.log(days);

			for (i in days) {
				self.playStack.push(days[i]);
			}

			delete days;

			//console.log(self.playStack);
		});

		delete months;

		if (self.playInterval == null) {
			self.playInterval = setInterval(function () {
				if (self.playStack.length > 0) {
					var data = self.playStack.shift(),
						id = 0;

					if (data !== undefined) {
						for (i in data) {
							var val = null,
								year = data[i].day.split('-')[0] | 0; // not very defensive, but heck with it...

							if (data[i].TAVG !== undefined) {
								val = data[i].TAVG.value;
							} else if (data[i].TMIN !== undefined && data[i].TMAX) {
								val = (data[i].TMIN.value + data[i].TMAX.value) / 2.0;
							}

							if (val != null) {
								self.transmit(id, val, 0, 0, 0, year);
							} else {
								self.transmit(id, 0, 0, 0, 1, year);
							}

							id++;
						}

						for (; id < self.channels; id++) {
							self.transmit(id, 0, 0, 0, 1)
						}
					}
				} else {
					self.log('clearing interval');
					clearInterval(self.playInterval);
					self.playInterval = null;
				}
			}, self.delay);
		}
	},

	transmit: function (oscillator, temp, precip, snowdepth, mute, year) {
		if (year !== undefined) {
			if (year !== this.lastYear) {
				this.lastYear = year;
				this.oscPort.send({
					address: '/year',
					args: [year]
				});
			} else {
				delete year;
			}

			console.log(year + ': ' + oscillator + ' ' + temp);
			this.oscPort.send({
				address: '/osc',
				args: [oscillator, temp, precip, snowdepth, mute]
			});
		}
	},

});

app.on('app:start', oscCatcher.init);

module.exports = oscCatcher;
