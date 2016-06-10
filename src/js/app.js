var ajax = require('ajax');

Pebble.addEventListener('ready', function(){
  console.log('PebbleKit JS ready!');
});

Pebble.addEventListener('showConfiguration', function(){
  var url = 'http://kennethschlatter.github.io/ASL-Config/index.html';
  console.log('Showing configuration page: ' + url);
  
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e){
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));
  
  var backgroundColor = configData.background_color;
  
  var dict = {};
  {
    dict.KEY_COLOR_RED = parseInt(backgroundColor.substring(2, 4), 16);
    dict.KEY_COLOR_GREEN = parseInt(backgroundColor.substring(4, 6), 16);
    dict.KEY_COLOR_BLUE = parseInt(backgroundColor.substring(6), 16);
  }
  
  // Send to Watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function(){
    console.log('Send failed. :(');
  });
});

// WEATHER STUFF

// Get location
function success(pos) {
  console.log('lat= ' + pos.coords.latitude + ' lon= ' + pos.coords.longitude);
  var lat = pos.coords.latitude;
  var lon = pos.coords.longitude;
}

function error(err) {
  console.log('location error (' + err.code + '): ' + err.message);
}

/* ... */

// Choose options about the data returned
var options = {
  enableHighAccuracy: true,
  maximumAge: 10000,
  timeout: 10000
};

// Request current position
navigator.geolocation.getCurrentPosition(success, error, options);

// Construct URL to get weather at
// Get your own API key at https://openweathermap.org
var apiKey = '50ef49bbe9fe20384c1756a17338d49c';
var URL = 'api.openweathermap.org/data/2.5/weather?lat=' + lat + '&lon=' + lon + '&appid=' + apiKey;
console.log(URL);

// Make the request
ajax(
  {
    url: URL,
    type: 'json'
  },
  function(data) {
    // Success!
    console.log('Successfully fetched weather data!');
    // Extract data
  var location = data.name;
  var temp_c = Math.round(data.main.temp - 273.15);
  var temp_f = Math.round(temp_c * 1.8 + 32);
  console.log('C: ' + temp_c + '   F: ' + temp_f + '   ' + location);
  // Always upper-case first letter of description
  var description = data.weather[0].description;
  description = description.charAt(0).toUpperCase() + description.substring(1);
},
  function(error) {
    // Failure!
    console.log('Failed fetching weather data: ' + error);
  }
);

// Assemble data object
var dict = {
  'KEY_TEMP_C': 'temp_c',
  'KEY_TEMP_F': 'temp_f',
  'KEY_LOCATION': 'location',
  'KEY_DESCRIPTION': 'description'
};

Pebble.sendAppMessage(dict, function() {
  console.log('Message sent successfully: ' + JSON.stringify(dict));
}, function(e) {
  console.log('Message failed: ' + JSON.stringify(e));
});