#The Sound of Change

##Prerequesites
For the POC version, which is what you see here now, you need:

- [Max/MSP](http://cycling74.com/)
- [Duration](http://duration.cc/)

to test the oscilators.  You'll also need to add the externals folder under max/ to your Max/MSP path (~/Documents/Max 7/Library by default, or add a path for the project's externals folder).

Weather data is available from [NOAA GHCN](ftp://ftp.ncdc.noaa.gov/pub/data/ghcn/daily/) project.  To download
into place using gulp, run `gulp data` and wait.

##To Do:

###Software
- Script data transform to usable daily values
- Create a data emitter to feed the oscillator from the data feed
- Scale oscillator side to deal with multiple data streams (ofx rather than max?)

###Deployment
Everything.