var RailViz = RailViz || {};

RailViz.Main = (function() {

  var apiEndpoint;
  var mapInfo;
  var elmPorts;
  var timeOffset = 0;
  var trainUpdateTimeoutId, tripsUpdateTimeoutId;
  var filteredTripIds;
  var fullData, filteredData;

  var hoverInfo = {x: -1, y: -1, pickedTrain: null, pickedStation: null};

  var debouncedSendTrainsRequest = debounce(sendTrainsRequest, 500);

  function init(canvas, endpoint, ports) {
    apiEndpoint = endpoint;
    elmPorts = ports;
    RailViz.Render.init(canvas, handleMouseEvent);
  }

  function mapUpdate(newMapInfo) {
    mapInfo = newMapInfo;
    RailViz.Render.setMapInfo(mapInfo);
    debouncedSendTrainsRequest();
  }

  function setTimeOffset(newTimeOffset) {
    timeOffset = newTimeOffset / 1000;
    RailViz.Render.setTimeOffset(timeOffset);
    debouncedSendTrainsRequest();
  }

  function setFilter(tripIds) {
    filteredTripIds = tripIds;
    if (filteredTripIds) {
      sendTripsRequest();
    } else {
      filteredData = null;
      RailViz.Render.setData(fullData);
    }
  }

  function makeTrainsRequest() {
    var bounds = mapInfo.railVizBounds;
    return RailViz.API.makeTrainsRequest(
        mapInfo.zoom, {lat: bounds.north, lng: bounds.west},
        {lat: bounds.south, lng: bounds.east}, timeOffset + (Date.now() / 1000),
        timeOffset + (Date.now() / 1000) + 120, 1000);
  }

  function makeTripsRequest() {
    return filteredTripIds && RailViz.API.makeTripsRequest(filteredTripIds);
  }

  function sendTrainsRequest() {
    if (trainUpdateTimeoutId) {
      clearTimeout(trainUpdateTimeoutId);
    }
    trainUpdateTimeoutId = setTimeout(debouncedSendTrainsRequest, 90000);
    RailViz.API.sendRequest(
        apiEndpoint, makeTrainsRequest(),
        response => dataReceived(response, false),
        response => { console.log('api error:', response); });
  }

  function sendTripsRequest() {
    if (tripsUpdateTimeoutId) {
      clearTimeout(tripsUpdateTimeoutId);
    }
    if (filteredTripIds) {
      tripsUpdateTimeoutId = setTimeout(sendTripsRequest, 90000);
      RailViz.API.sendRequest(
          apiEndpoint, makeTripsRequest(),
          response => dataReceived(response, true),
          response => { console.log('api error:', response); });
    }
  }

  function dataReceived(response, onlyFilteredTrips) {
    var data = response.content;
    RailViz.Preprocessing.preprocess(data);
    if (onlyFilteredTrips) {
      filteredData = data;
      if (filteredTripIds) {
        RailViz.Render.setData(data);
      }
    } else {
      fullData = data;
      if (!filteredTripIds) {
        RailViz.Render.setData(data);
      }
    }
  }

  function handleMouseEvent(eventType, x, y, pickedTrain, pickedStation) {
    if (eventType == 'mousedown') {
      if (pickedTrain) {
        if (pickedTrain.trip && pickedTrain.trip.length > 0) {
          elmPorts.showTripDetails.send(pickedTrain.trip[0]);
        }
      } else if (pickedStation) {
        elmPorts.showStationDetails.send(pickedStation.id);
      }
    } else {
      if (eventType != 'mouseout') {
        setTooltip(x, y, pickedTrain, pickedStation);
      } else {
        setTooltip(-1, -1, null, null);
      }
    }
  }

  function setTooltip(x, y, pickedTrain, pickedStation) {
    if (hoverInfo.x == x && hoverInfo.y == y &&
        hoverInfo.pickedTrain == pickedTrain &&
        hoverInfo.pickedStation == pickedStation) {
      return;
    }

    hoverInfo.x = x;
    hoverInfo.y = y;
    hoverInfo.pickedTrain = pickedTrain;
    hoverInfo.pickedStation = pickedStation;

    var rvTrain = null;
    if (pickedTrain) {
      rvTrain = {
        names: pickedTrain.names,
        departureTime: pickedTrain.d_time * 1000,
        arrivalTime: pickedTrain.a_time * 1000,
        scheduledDepartureTime: pickedTrain.sched_d_time * 1000,
        scheduledArrivalTime: pickedTrain.sched_a_time * 1000,
        hasDepartureDelayInfo:
            !!(pickedTrain.d_time_reason &&
               pickedTrain.d_time_reason != 'SCHEDULE'),
        hasArrivalDelayInfo:
            !!(pickedTrain.a_time_reason &&
               pickedTrain.a_time_reason != 'SCHEDULE'),
        departureStation: pickedTrain.departureStation.name,
        arrivalStation: pickedTrain.arrivalStation.name
      };
    }
    var rvStation = pickedStation && pickedStation.name;

    elmPorts.mapSetTooltip.send({
      mouseX: x,
      mouseY: y,
      hoveredTrain: rvTrain,
      hoveredStation: rvStation
    });
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
    setFilter: setFilter
  };

})();