var CanvasOverlay = L.Layer.extend({
  initialize: function() {
    L.setOptions(this, {});
  },

  onAdd: function(map) {
    map._panes.overlayPane.appendChild(this._el);

    setTimeout(function() {
      this._updateSize();
    }.bind(this), 100);
  },

  onRemove: function(map) {
    map.getPanes().overlayPane.removeChild(this._el);
  },

  getEvents: function() {
    var events = {
      dragend: this._dragEnd,
      move: this._updatePosition,
      moveend: this._updatePosition,
      resize: this._updateSize,
      zoom: this._zoom,
      zoomend: this._zoomEnd
    };

    if (this._zoomAnimated) {
      events.zoomanim = this._animateZoom;
    }

    return events;
  },

  _animateZoom: function(e) {
    this._updateTransform(e.center, e.zoom);
  },

  _zoom: function() {
    this._updateTransform(this._map.getCenter(), this._map.getZoom());
  },

  _zoomEnd: function() {
    this._update();
  },

  _dragEnd: function() {
    RailViz.Main.dragEnd();
    this._updatePosition();
  },

  _updateTransform: function(center, zoom) {
    var scale = this._map.getZoomScale(zoom),
        position = L.DomUtil.getPosition(this._el),
        viewHalf = this._map.getSize().multiplyBy(0.5),
        currentCenterPoint = this._map.project(this._map.getCenter(), zoom),
        destCenterPoint = this._map.project(center, zoom),
        centerOffset = destCenterPoint.subtract(currentCenterPoint),
        topLeftOffset =
            viewHalf.multiplyBy(-scale).add(position).add(viewHalf).subtract(
                centerOffset);
    L.DomUtil.setTransform(this._el, topLeftOffset, scale);
  },

  _update: function() {
    var pixelBounds = this._map.getPixelBounds().min;
    var geoBounds = this._map.getBounds();
    var size = this._map.getSize();
    var center = this._map.getCenter();
    // var railVizBounds = L.latLngBounds(
    //     this._map.unproject(pixelBounds.subtract(size)),
    //     this._map.unproject(pixelBounds.add(size).add(size)));
    var railVizBounds = geoBounds;

    var zoom = Math.round(this._map.getZoom());
    var mapInfo = {
      scale: Math.pow(2, zoom),
      zoom: zoom,
      pixelBounds: {
        north: pixelBounds.y,
        west: pixelBounds.x,
        width: size.x,
        height: size.y
      },
      geoBounds: {
        north: geoBounds.getNorth(),
        west: geoBounds.getWest(),
        south: geoBounds.getSouth(),
        east: geoBounds.getEast()
      },
      railVizBounds: {
        north: railVizBounds.getNorth(),
        west: railVizBounds.getWest(),
        south: railVizBounds.getSouth(),
        east: railVizBounds.getEast()
      },
      center: {lat: center.lat, lng: center.lng}
    };

    app.ports.mapUpdate.send(mapInfo);
    RailViz.Main.mapUpdate(mapInfo);
  },

  _updateSize: function() {
    var size = this._map.getSize();
    this._el.style.width = size.x + 'px';
    this._el.style.height = size.y + 'px';
    this._updatePosition();
  },

  _updatePosition: function() {
    this._update();
    var pos = this._map.containerPointToLayerPoint([0, 0]);
    L.DomUtil.setPosition(this._el, pos);
  }

});

function initPorts(app, apiEndpoint) {
  app.ports.mapInit.subscribe(function(id) {
    var map = L.map('map', {zoomControl: false}).setView([49.8728, 8.6512], 14);

    L.tileLayer(
         'https://tiles.motis-project.de/osm_light/{z}/{x}/{y}.png?token={accessToken}',
         {
           attribution: 'Map data &copy; OpenStreetMap contributors, CC-BY-SA',
           maxZoom: 18,
           accessToken:
               '862bdec137edd4e88029304609458291f0ec760b668c5816ccdd83d0beae76a4'
         })
        .addTo(map);

    L.control.zoom({position: 'topright'}).addTo(map);

    window.elmMaps[id] = map;

    var c = new CanvasOverlay();
    c._el = map.getContainer().querySelector('.railviz-overlay');
    map.addLayer(c);

    RailViz.Main.init(c._el, apiEndpoint, app.ports);
  });

  app.ports.setRailVizFilter.subscribe(RailViz.Main.setTripFilter);
  app.ports.mapSetConnectionFilter.subscribe(RailViz.Main.setConnectionFilter);
  app.ports.setTimeOffset.subscribe(RailViz.Main.setTimeOffset);

  app.ports.mapFlyTo.subscribe(function(opt) {
    var map = window.elmMaps[opt.mapId];
    var center = L.latLng(opt.lat, opt.lng);
    var options = {animate: opt.animate};
    if (opt.zoom) {
      map.flyTo(center, opt.zoom, options);
    } else {
      Object.assign(options, {paddingTopLeft: [600, 0], maxZoom: 16});
      map.flyToBounds(L.latLngBounds([center]), options);
    }
  });

  app.ports.mapFitBounds.subscribe(function(opt) {
    var map = window.elmMaps[opt.mapId];
    var bounds = L.latLngBounds(opt.coords);
    map.fitBounds(bounds, {paddingTopLeft: [600, 0]});
  });

  app.ports.mapUseTrainClassColors.subscribe(
      RailViz.Trains.setUseCategoryColor);
}
