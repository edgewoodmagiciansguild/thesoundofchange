// Create application
var EventEmitter = require('events').EventEmitter
	app = module.exports = new EventEmitter();

// Load utilities
var _ = require('lodash'),
	requireAll = require('require-directory');

// Load configuration
app.config = require('config');

app.name = app.config.name;

// Load libraries
_.assign(app, requireAll(module, './lib'));

// Load application modules
_.assign(app, requireAll(module));

global.Errors = app.errors;

// Start
app.emit('app:start');
