var RailViz = RailViz || {};

RailViz.Main = (function() {

  var apiEndpoint;
  var mapInfo;
  var elmPorts;
  var timeOffset = 0;

  var debouncedSendRequest = debounce(sendRequest, 500);

  function init(canvas, endpoint, ports) {
    apiEndpoint = endpoint;
    elmPorts = ports;
    RailViz.Render.init(canvas, handleMouseEvent);
  }

  function mapUpdate(newMapInfo) {
    mapInfo = newMapInfo;
    RailViz.Render.setMapInfo(mapInfo);
    var bounds = mapInfo.railVizBounds;
    var request = RailViz.API.makeTrainsRequest(
        {lat: bounds.north, lng: bounds.west},
        {lat: bounds.south, lng: bounds.east}, timeOffset + (Date.now() / 1000),
        timeOffset + (Date.now() / 1000) + 120, 1000);
    debouncedSendRequest(request);
  }

  function setTimeOffset(newTimeOffset) { timeOffset = newTimeOffset; }

  function sendRequest(request) {
    RailViz.API.sendRequest(apiEndpoint, request, dataReceived, response => {
      console.log('api error:', response);
    });
  }

  function dataReceived(response) {
    var data = response.content;
    RailViz.Preprocessing.preprocess(data);
    RailViz.Render.setData(data);
  }

  function handleMouseEvent(eventType, x, y, pickedTrain, pickedStation) {
    if (eventType == 'mousedown') {
      if (pickedTrain) {
        console.log('clicked train:', pickedTrain);
        if (pickedTrain.trip && pickedTrain.trip.length > 0) {
          elmPorts.showTripDetails.send(pickedTrain.trip[0]);
        }
      } else if (pickedStation) {
        console.log('clicked station:', pickedStation);
        elmPorts.showStationDetails.send(
            [pickedStation.id, pickedStation.name]);
      }
    }
  }

  function debounce(f, t) {
    var timeout;
    return function() {
      var context = this;
      var args = arguments;
      var later = function() {
        timeout = null;
        f.apply(context, args);
      };
      clearTimeout(timeout);
      timeout = setTimeout(later, t);
    }
  }

  return {
    init: init,
    debounce: debounce,
    mapUpdate: mapUpdate,
    setTimeOffset: setTimeOffset,
  };

})();