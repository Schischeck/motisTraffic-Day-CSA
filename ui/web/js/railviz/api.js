var RailViz = RailViz || {};

RailViz.API = (function() {

  var lastCallId = 0;

  function makeTrainsRequest(
      zoom_level, corner1, corner2, startTime, endTime, maxTrains) {
    return {
      destination: {type: 'Module', target: '/railviz/get_trains'},
      content_type: 'RailVizTrainsRequest',
      content: {
        zoom_level: zoom_level,
        corner1: corner1,
        corner2: corner2,
        start_time: Math.floor(startTime),
        end_time: Math.ceil(endTime),
        max_trains: maxTrains
      }
    };
  }

  function makeTripsRequest(tripIds) {
    return {
      destination: {type: 'Module', target: '/railviz/get_trips'},
      content_type: 'RailVizTripsRequest',
      content: {trips: tripIds}
    };
  }

  function sendRequest(apiEndpoint, requestData, onSuccess, onFail) {
    var xhr = new XMLHttpRequest();
    const callId = ++lastCallId;
    const startTime = performance.now();
    xhr.addEventListener('load', function() {
      var response = xhr.responseText;
      try {
        response = JSON.parse(xhr.responseText);
      } catch (ex) {
      }
      if (xhr.status == 200) {
        onSuccess(response, callId, performance.now() - startTime);
      } else {
        onFail(response, callId, performance.now() - startTime);
      }
    });
    xhr.addEventListener('error', function() {
      onFail('NetworkError', callId, performance.now() - startTime);
    });
    xhr.addEventListener('timeout', function() {
      onFail('TimeoutError', callId, performance.now() - startTime);
    })
    xhr.open('POST', apiEndpoint);
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.send(JSON.stringify(requestData));
    return callId;
  }

  return {
    makeTrainsRequest: makeTrainsRequest,
    makeTripsRequest: makeTripsRequest,
    sendRequest: sendRequest,
  };
})();
