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

	init: function() {
		var self = this;

		self.log = app.logger.create('noaaXmlProcessor');
		self.error = app.logger.create('noaaXmlProcessor', 'error');
		self.warn = app.logger.create('noaaXmlProcessor', 'warn');

		self.log('init');

		app.on('noaa:data', function (station, data) {
			console.log({station: station, data: data});
		});

		self.processStations(path.join(self.path, self.stationFile)).then(function (stations) {
			self.stations = stations;

			//console.log(stations);

			self.processPath(path.join(self.path, self.dailyPath), function (file) {
				return self.pattern.test(file);
			}, function (fileQueue) {
				self.files = fileQueue;

				self.processFiles();
			});
		});
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
			chain = [];

		for (var i in self.files) {
			chain.push(self.processFile(self.files[i]));
		}

		return Promise.all(chain);
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

			reader.on('line', function (line) {
				line = line.replace(/\s{2,}/g, ' ');
				line = line.replace(/(\w)(\-)([0-9])/g, '$1 -$3')

				var parts = line.trim().split(/\s/);
				if (parts.length > 0) {
					var station = parts[0].substr(0, 11),
						year = parts[0].substr(11, 4),
						month = parts[0].substr(15, 2),
						type = parts[0].substr(17, 4),
						id = year + '-' + month,
						date = new Date(parseInt(year), parseInt(month) - 1);

					if (lastId == null || lastId != id) {
						if (lastId != id) {
							app.emit('noaa:data', self.stations[station], data);
						}

						lastId = id;
						data = {};
						data[id] = {};
						data[id].date = date;
					}

					data[id][type] = parts.splice(1, parts.length - 1);
				}
			});
		});
	}

});

app.on('app:start', noaaXmlProcessor.init);

module.exports = noaaXmlProcessor;
