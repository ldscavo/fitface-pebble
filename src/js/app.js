var Clay = require('pebble-clay');
var clayConfig = require('./config.js');
var clay = new Clay(clayConfig);

var location;

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    
    getWeather();
  }                     
);

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  sendWeatherRequest("lat=" + pos.coords.latitude + '&lon=' + pos.coords.longitude);
}

function locationError(err) {
  console.log('Error requesting location!');
}

function getWeather() {
  navigator.geolocation.getCurrentPosition(
    locationSuccess,
    locationError,
    {timeout: 15000, maximumAge: 60000}
  );
}

function sendWeatherRequest(queryStringParams) {
  var myAPIKey = '499b4f8b067ddc0eac377f41fd5c7a7e';
  
  // Construct URL
  var url = 'http://api.openweathermap.org/data/2.5/weather?units=imperial&appid=' + myAPIKey;  
  url += "&" + queryStringParams;
    
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      console.log(responseText);
      // Temperature in Kelvin requires adjustment
      var temperature = json.main.temp;

      // Assemble dictionary using our keys
      var dictionary = {
        'KEY_TEMP': temperature
      };
      
      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log('Weather info sent to Pebble successfully!');
        },
        function(e) {
          console.log('Error sending weather info to Pebble!');
        }
      );
    }      
  );
}

function getLocalWeather() {
  sendWeatherRequest("q=" + location);
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    
    location = JSON.parse(localStorage.getItem('clay-settings'));
    if (location === null) {
      location = "";
    } else {
      location = location.LOCATION;
    }
        
    // Get the initial weather
    if (location === "") {
      getWeather();
    } else {
      getLocalWeather();
    }    
  }
);