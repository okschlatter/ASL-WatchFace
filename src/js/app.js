//Add OWM Weather
var OWMWeather = require('pebble-owm-weather');
var owmWeather = new OWMWeather();

//Add Clay Config Page
var Clay = require('pebble-clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

Pebble.addEventListener('ready', function(){
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('appmessage', function(e) {
  console.log('appmessage: ' + JSON.stringify(e.payload));
  owmWeather.appMessageHandler(e);
});