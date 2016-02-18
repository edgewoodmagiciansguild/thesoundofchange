// Get link to main module
app = require.main.exports;

// Load required modules
var _ = require('lodash'),
	fs = require('fs')
	path = require('path');

var noaaXmlProcessor = _.bindAll({

	path: app.config.noaa.dataDir,
	pattern: new RegExp('^.+\.dly$', 'i'),
	files: [],

	init: function() {
		var self = this;

		self.log = app.logger.create('noaaXmlProcessor');
		self.error = app.logger.create('noaaXmlProcessor', 'error');
		self.warn = app.logger.create('noaaXmlProcessor', 'warn');

		self.log('init');

		self.processPath(self.path, function (file) {
			return self.pattern.test(file);
		}, function (fileQueue) {
			self.files = fileQueue;
			console.log(self.files);
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

	},

	processFile: function (path) {

		var parser = new htmlparser.Parser(new htmlparser.DefaultHandler(function (error, dom) {
			if (!error) {
				for (var i in dom) {
					console.log(dom[i]);
				}
			} else {
				self.error(error);
			}
		}, { ignoreWhitespace: true }));

		parser.parseComplete(data);
	}

});

app.on('app:start', noaaXmlProcessor.init);

module.exports = noaaXmlProcessor;
