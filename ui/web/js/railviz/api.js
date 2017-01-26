var RailViz = RailViz || {};

RailViz.API = (function() {

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
    xhr.onreadystatechange = function() {
      if (xhr.readyState == XMLHttpRequest.DONE) {
        var response = xhr.responseText && JSON.parse(xhr.responseText);
        if (xhr.status == 200) {
          onSuccess(response);
        } else {
          onFail(response);
        }
      }
    };
    xhr.open('POST', apiEndpoint);
    xhr.setRequestHeader('Content-Type', 'application/json');
    xhr.send(JSON.stringify(requestData));
  }

  return {
    makeTrainsRequest: makeTrainsRequest,
    makeTripsRequest: makeTripsRequest,
    sendRequest: sendRequest,
  };

})();
