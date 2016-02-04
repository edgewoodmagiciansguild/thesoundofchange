var gulp = require('gulp'),
	gutil = require('gulp-util'),
	ftp = require('vinyl-ftp'),
	gunzip = require('gulp-gunzip'),
	untar = require('gulp-untar'),
	del = require('del'),
	conn = ftp.create( {
			host: 'ftp.ncdc.noaa.gov',
			user: 'anonymous',
			pass: 'kurt.roberts@gmail.com', 
			parallel: 10,
			log:      gutil.log
		}),
	prefix = '/pub/data/ghcn/daily/',
	files = [
			prefix + 'data-readme.txt',
			prefix + 'ghcnd-stations.txt',
			prefix + 'ghcnd_gsn.tar.gz'
	],
	paths = {
		data: __dirname + '/data/',
		tarxvf: __dirname + '/data/ghcnd_gsn.tar.gz'
	};

gulp.task('clean', function () {
	return del([paths.data]);
});


gulp.task('get-data', function () {
	return conn.src(files)
			.pipe(gulp.dest(paths.data) );

} );

gulp.task('data', ['get-data'], function () {
	return gulp.src(paths.tarxvf)
			.pipe(gunzip())
			.pipe(untar())
			.pipe(gulp.dest(paths.data));
});