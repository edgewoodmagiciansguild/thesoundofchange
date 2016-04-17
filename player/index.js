var _ = require('lodash'),
	fs = require('fs')
	path = require('path')
	readline = require('readline'),
	Promise = require('bluebird'),
	osc = require('osc');

// Load configuration
app = {};
app.config = require('config');

var player = _.bindAll({

	pattern: new RegExp('^.+\.json$', 'i'),
	dataPath: 'processor/data/',
	files: [],

	playStack: [],
	playInterval: null,
	oscPort: null,
	delay: app.config.oscCatcher.delay,

	currentDay: null,
	lastYear: null,

	oscConfig: {
		localAddress: '0.0.0.0',
		localPort: 5001,
		remoteAddress: app.config.udpEmitter.host,
		remotePort: app.config.udpEmitter.port
	},

	init: function () {
		var self = this;

		self.processPath(self.dataPath, function (file) {
			return self.pattern.test(file);
		}, function (fileQueue) {
			self.files = fileQueue;

			self.play();
		});
	},

	processPath: function(rootPath, filter, callback, fileQueue) {
		var self = this,
			files = fs.readdirSync(rootPath),
			isFilterable = (typeof filter === 'function');

		fileQueue = (fileQueue || []);

		for (var i = 0; i < files.length; i++) {
			var fullPath = path.join(rootPath, files[i]);

			if (fs.statSync(fullPath).isDirectory()) {
				self.processPath(fullPath, filter, undefined, fileQueue);
			} else if (isFilterable && filter(files[i])) {
				fileQueue.push(fullPath);
			}
		}

		if (typeof callback === 'function') {
			callback(fileQueue);
		}
	},

	getNextSet: function () {
		var self = this;

		if (self.files.length > 0) {
			var file = self.files.shift(),
				set = JSON.parse(fs.readFileSync(file, 'utf8'));
				queue = [];

			for (var m in set) {
				for (var s in set[m]) {
					set[m][s].day = m;
				}

				queue.push(set[m]);
			}

			return queue;
		}
	},

	play: function () {
		var self = this;

		self.oscPort = new osc.UDPPort(self.oscConfig);
		self.oscPort.open();

		if (self.playInterval == null) {
			self.playInterval = setInterval(function () {
				if (self.playStack.length > 0) {
					var data = self.playStack.shift(),
						id = 0;

					if (data !== undefined) {
						for (i in data) {
							var tmpVal = null,
								prcpVal = null,
								snowVal = null,
								year = parseInt(data[i].day.split('-')[0] || 0); // not very defensive, but heck with it... KPR: needed an int, osc was sending a string without the cast

							if (data[i].TAVG !== undefined) {
								tmpVal = data[i].TAVG.value;
							} else if (data[i].TMIN !== undefined && data[i].TMAX) {
								tmpVal = (data[i].TMIN.value + data[i].TMAX.value) / 2.0;
							}

							if (data[i].PRCP !== undefined) {
								prcpVal = data[i].PRCP.value;
							}

							if (data[i].SNOW !== undefined) {
								snowVal = data[i].SNOW.value;
							}

							if (tmpVal != null || prcpVal != null || snowVal != null) {
								self.transmit(id, tmpVal, prcpVal, snowVal, 0, year);
							} else {
								self.transmit(id, 0, 0, 0, 1, year);
							}

							id++;
						}

						self.oscPort.send({
							address: '/activeVoices',
							args: [Object.keys(data).length]
						});

						for (; id < self.channels; id++) {
							self.transmit(id, 0, 0, 0, 1);
						}

					}
				} else {
					self.playStack = self.getNextSet();
					if (self.playStack.length === 0) {
						console.log('clearing interval');
						clearInterval(self.playInterval);
						self.playInterval = null;

						self.oscPort.close();
					}
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

			console.log(year + ': ' + oscillator + ' ' + temp + ' ' + precip + ' ' + snowdepth);
			this.oscPort.send({
				address: '/osc',
				args: [oscillator, temp || 'null', precip || 'null', snowdepth || 'null', mute]
			});
		}
	},

});

player.init();

module.exports = player;