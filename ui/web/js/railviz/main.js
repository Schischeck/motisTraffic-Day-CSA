var RailViz = RailViz || {};

RailViz.Main = (function() {

  var apiEndpoint;
  var mapInfo;
  var elmPorts;
  var timeOffset = 0;
  var trainUpdateTimeoutId, tripsUpdateTimeoutId;
  var filteredTripIds;
  var connectionFilter = null;
  var fullData, filteredData;
  var showingFilteredData = false;
  var dragEndTime = null;
  var lastTrainsRequest = null;
  var lastTripsRequest = null;
  var useFpsLimiter = true;

  var hoverInfo = {x: -1, y: -1, pickedTrain: null, pickedStation: null};

  var debouncedSendTrainsRequest = debounce(sendTrainsRequest, 200);

  const FILTERED_MIN_ZOOM = 14;

  function init(canvas, endpoint, ports) {
    apiEndpoint = endpoint;
    elmPorts = ports;
    RailViz.Render.init(canvas, handleMouseEvent);
  }

  function mapUpdate(newMapInfo) {
    mapInfo = newMapInfo;
    RailViz.Render.setMapInfo(mapInfo);
    setupFpsLimiter();
    debouncedSendTrainsRequest();
  }

  function setTimeOffset(newTimeOffset) {
    timeOffset = newTimeOffset / 1000;
    RailViz.Render.setTimeOffset(timeOffset);
    debouncedSendTrainsRequest();
  }

  function setTripFilter(tripIds) {
    filteredTripIds = tripIds;
    if (filteredTripIds) {
      sendTripsRequest();
    } else {
      filteredData = null;
      showFullData();
    }
  }

  function setConnectionFilter(filter) {
    filter.walks.forEach(RailViz.Preprocessing.prepareWalk);
    connectionFilter = filter;
    if (showingFilteredData) {
      RailViz.Render.setConnectionFilter(connectionFilter);
    }
  }

  function setUseFpsLimiter(enabled) {
    useFpsLimiter = enabled;
    setupFpsLimiter();
  }

  function setupFpsLimiter() {
    let targetFps = null;
    if (useFpsLimiter && mapInfo) {
      const zoom = mapInfo.zoom;
      if (zoom <= 10) {
        targetFps = 2;
      } else if (zoom <= 12) {
        targetFps = 5;
      } else if (zoom <= 14) {
        targetFps = 10;
      } else if (zoom <= 15) {
        targetFps = 30;
      }
    }
    RailViz.Render.setTargetFps(targetFps);
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
    lastTrainsRequest = RailViz.API.sendRequest(
        apiEndpoint, makeTrainsRequest(),
        (response, callId, duration) =>
            dataReceived(response, false, callId, duration),
        response => {
          elmPorts.handleRailVizError.send(response);
        });
  }

  function sendTripsRequest() {
    if (tripsUpdateTimeoutId) {
      clearTimeout(tripsUpdateTimeoutId);
    }
    if (filteredTripIds) {
      tripsUpdateTimeoutId = setTimeout(sendTripsRequest, 90000);
      lastTripsRequest = RailViz.API.sendRequest(
          apiEndpoint, makeTripsRequest(),
          (response, callId, duration) =>
              dataReceived(response, true, callId, duration),
          response => {
            elmPorts.handleRailVizError.send(response);
          });
    }
  }

  function dataReceived(response, onlyFilteredTrips, callId, duration) {
    var data = response.content;
    const lastRequest =
        onlyFilteredTrips ? lastTripsRequest : lastTrainsRequest;
    if (callId != lastRequest) {
      return;
    }
    RailViz.Preprocessing.preprocess(data);
    if (onlyFilteredTrips) {
      filteredData = data;
      if (filteredTripIds) {
        showFilteredData();
      }
    } else {
      fullData = data;
      if (!filteredTripIds) {
        showFullData();
      }
    }
    elmPorts.clearRailVizError.send(null);
  }

  function showFullData() {
    showingFilteredData = false;
    RailViz.Render.setData(fullData);
    RailViz.Render.setMinZoom(0);
  }

  function showFilteredData() {
    showingFilteredData = true;
    RailViz.Render.setData(filteredData);
    RailViz.Render.setMinZoom(FILTERED_MIN_ZOOM);
    RailViz.Render.setConnectionFilter(connectionFilter);
    RailViz.Render.colorRouteSegments();
  }

  function handleMouseEvent(eventType, x, y, pickedTrain, pickedStation) {
    if (eventType == 'mouseup') {
      if (dragEndTime != null && (Date.now() - dragEndTime < 100)) {
        // ignore mouse up events immediately after drag end
        return;
      }
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

  function dragEnd() {
    dragEndTime = Date.now();
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
    getTimeOffset: function() {
      return timeOffset;
    },
    setTripFilter: setTripFilter,
    setConnectionFilter: setConnectionFilter,
    dragEnd: dragEnd,
    setUseFpsLimiter: setUseFpsLimiter
  };

})();
