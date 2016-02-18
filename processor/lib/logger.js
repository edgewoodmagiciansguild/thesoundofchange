/**
* Logger Library
*/

// Get link to main module
app = require.main.exports;

// Load required modules
var _ = require('lodash'),
	debug = require('debug');

// Create Logger singleton
var logger = _.bindAll({

	init: function() {
		// Override debug namespaces depending on configs
		if(app.config.debug === true) {
			debug.enable(process.env.DEBUG || app.name + ':*');
		} else if(app.config.debug !== false && _.isString(app.config.debug)) {
			debug.enable(app.config.debug);
		} else {
			debug.enable(app.name + ':*-error ' + app.name + ':*-warn');
		}

		app.log = this.create('app');
		app.error = this.create('app', 'error');
		app.warn = this.create('app', 'warn');

	},

	create: function(namespace, level) {
		if(level && console[level]) {
			var func = debug(app.name + ':' + namespace + '-' + level);
			func.log = _.bind(console[level], console);
			return func;
		}
		return debug(app.name + ':' + namespace);
	},

});

// Initialize logger immediately
logger.init();

// Export logger library
module.exports = logger;
