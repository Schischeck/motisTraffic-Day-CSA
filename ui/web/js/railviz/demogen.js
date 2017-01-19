var RailViz = RailViz || {};

RailViz.DemoGenerator = (function() {

  var width;
  var height;

  function generate(n_stations, n_routes, n_trains, w, h) {
    var stations = [];
    var routes = [];
    var trains = [];

    width = w || 1024;
    height = h || 768;

    generateStations(stations, n_stations);
    generateRoutes(routes, n_routes, stations);
    generateTrains(trains, n_trains, routes);

    return {stations: stations, routes: routes, trains: trains};
  }

  function generateStations(stations, count) {
    for (var i = 0; i < count; i++) {
      stations[i] = {
        id: 'st' + i,
        name: 'Station #' + i,
        pos: {x: Math.random() * width, y: Math.random() * height}
      };
    }
  }

  function generateRoutes(routes, count, stations) {
    for (var i = 0; i < count; i++) {
      var n_segments = Math.floor(Math.random() * 5 + 1);
      var segments = [];
      var from = getRandom(stations);
      for (var j = 0; j < n_segments; j++) {
        var to = getRandom(stations);
        while (to.id == from.id) {
          to = getRandom(stations);
        }
        var samples = 10;
        var polyline = generatePolyline(from.pos, to.pos, samples);
        segments[j] = {
          from_station_id: from.id,
          to_station_id: to.id,
          coordinates: {coordinates: polyline}
        };
        from = to;
      }
      routes[i] = {segments: segments};
    }
  }

  function generatePolyline(from, to, samples) {
    var boundingBox = {
      left: Math.min(from.x, to.x) - 10,
      top: Math.min(from.y, to.y) - 10,
      right: Math.max(from.x, to.x) + 10,
      bottom: Math.max(from.y, to.y) + 10
    };
    var ctrl1 = randomPoint(boundingBox);
    var ctrl2 = randomPoint(boundingBox);
    var coords = new Array((samples + 1) * 2);
    var delta = 1.0 / samples;
    for (var i = 0; i <= samples; i++) {
      const point = cbezier(from, ctrl1, ctrl2, to, delta * i);
      coords[i * 2] = point.x;
      coords[i * 2 + 1] = point.y;
    };
    return coords;
  }

  function generateTrains(trains, count, routes) {
    for (var i = 0; i < count; i++) {
      var routeIdx = getRandomIndex(routes);
      var segmentIdx = getRandomIndex(routes[routeIdx].segments);
      var d_time = Math.floor(Math.random() * 20);
      var a_time = d_time + 10 + Math.floor(Math.random() * 60);
      trains[i] = {
        names: ['DT ' + i],
        d_time: d_time,
        a_time: a_time,
        sched_d_time: d_time,
        sched_a_time: a_time,
        route_index: routeIdx,
        segment_index: segmentIdx,
        trip: []
      };
    }
  }

  function getRandom(array) { return array[getRandomIndex(array)]; }

  function getRandomIndex(array) {
    return Math.floor(Math.random() * array.length);
  }

  function randomPoint(boundingBox) {
    return {
      x: boundingBox.left +
          Math.random() * (boundingBox.right - boundingBox.left),
      y: boundingBox.top +
          Math.random() * (boundingBox.bottom - boundingBox.top)
    };
  }

  function cbezier(w0, w1, w2, w3, t) {
    const t2 = t * t;
    const t3 = t2 * t;
    const mt = 1.0 - t;
    const mt2 = mt * mt;
    const mt3 = mt2 * mt;

    return {
      x: mt3 * w0.x + (3 * mt2 * t) * w1.x + (3 * mt * t2) * w2.x + t3 * w3.x,
      y: mt3 * w0.y + (3 * mt2 * t) * w1.y + (3 * mt * t2) * w2.y + t3 * w3.y
    };
  }

  return {generate: generate};

})();
