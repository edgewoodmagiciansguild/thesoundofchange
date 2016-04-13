// Get link to main module
app = require.main.exports;

// Load required modules
var _ = require('lodash'),
	fs = require('fs')
	path = require('path')
	readline = require('readline'),
	Promise = require('bluebird');

var noaaXmlProcessor = _.bindAll({

	path: app.config.noaa.dataDir,
	dailyPath: 'data/ghcnd_gsn',
	stationFile: 'ghcnd-stations.txt',
	pattern: new RegExp('^.+\.dly$', 'i'),
	files: [],

	isPaused: false,
	resume: null,

	init: function() {
		var self = this;

		self.log = app.logger.create('noaaXmlProcessor');
		self.error = app.logger.create('noaaXmlProcessor', 'error');
		self.warn = app.logger.create('noaaXmlProcessor', 'warn');

		self.log('init');

		self.processStations(path.join(self.path, self.stationFile)).then(function (stations) {
			self.stations = stations;

			self.processPath(path.join(self.path, self.dailyPath), function (file) {
				return self.pattern.test(file);
			}, function (fileQueue) {
				self.files = fileQueue;

				self.processFiles().then(function () {
					self.log('done');
					app.emit('noaa:done');
				});
			});
		});

		app.on('noaa:pause', self.pause);
		app.on('noaa:resume', self.resume);
	},

	pause: function () {
		this.isPaused = true;
	},

	resume: function () {
		var r = this.resume;

		if (r != null) {
			this.log('resuming');
			this.isPaused = false;
			this.resume = null;

			r();
		}
	},

	processStations: function (path) {
		var self = this,
			stations = {};

		self.log('processing stations');

		return new Promise(function(resolve, reject) {
			var input = fs.createReadStream(path),
				reader = readline.createInterface({
					input: input
				});

			reader.on('line', function (line) {
				line = line.replace(/([a-z\(\)\-\.])(\s)([a-z\(\)\-\.])/gi, '$1_$3');
				line = line.replace(/\s{2,}/g, ' ');

				var parts = line.trim().split(/\s/);

				if (parts.length > 0) {
					stations[parts[0]] = {};
					stations[parts[0]].id = parts[0];

					if (parts.length >= 2) {
						stations[parts[0]].lat = parts[1];
					}

					if (parts.length >= 3) {
						stations[parts[0]].lon = parts[2];
					}

					if (parts.length >= 4) {
						stations[parts[0]].alt = parts[3];
					}

					if (parts.length >= 5) {
						stations[parts[0]].name = parts[4].replace(/_/g, ' ');
					}
				}
			});

			reader.on('close', function () {
				input.destroy();
				delete input;
				delete reader;

				resolve(stations);
			});
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

	processFiles: function () {
		var self = this,
			chain = Promise.cast();

		function next(chain) {
			return chain.then(function () {
				if (self.files.length > 0) {
					file = self.files.shift();
					return next(self.processFile(file));
				}
			})
		}

		return next(chain);
	},

	processFile: function (path) {
		var self = this;

		return new Promise(function(resolve, reject) {
			var input = fs.createReadStream(path),
				reader = readline.createInterface({
					input: input
				}),
				data = {},
				lastId = null;

			self.log('processing ' + path);

			reader.on('line', function (line) {
				var station = line.substr(0, 11),
					year = line.substr(11, 4),
					month = line.substr(15, 2),
					type = line.substr(17, 4),
					id = year + '-' + month,
					date = new Date(parseInt(year), parseInt(month) - 1),
					days = [];

				if (lastId == null || lastId != id) {
					if (lastId != id && lastId != null) {
						app.emit('noaa:data', self.stations[station], data);
					}

					lastId = id;
					data = {};
					data.id = id;
					data.date = date;
				}

				for (var offset = 21; offset < line.length; offset += 8) {
					var dayData = line.substr(offset, 8),
						val = parseInt(dayData.substr(0, 5)),
						measure = dayData.substr(5, 1),
						quality = dayData.substr(6, 1)
						source = dayData.substr(7, 1);

					if (val === -9999) {
						val = NaN;
					}

					days.push({
						value: val,
						measure: (measure === ' ' ? null : measure),
						quality: (quality === ' ' ? null : quality),
						source: (source === ' ' ? null : source)
					});
				}

				data[type] = days;
			});

			reader.on('close', function () {
				input.destroy();
				delete input;
				delete reader;

				if (self.isPaused) {
					self.log('paused');
					self.resume = resolve;
					app.emit('noaa:paused');
				} else {
					resolve();
				}
			});
		});
	}

});

app.on('app:start', noaaXmlProcessor.init);

module.exports = noaaXmlProcessor;
