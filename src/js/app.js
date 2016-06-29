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