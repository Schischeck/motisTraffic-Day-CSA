var CanvasOverlay = L.Layer.extend({
  initialize: function() { L.setOptions(this, {}); },

  onAdd: function(map) {
    this._map = map;

    map._panes.overlayPane.appendChild(this._el);

    map.on('dragend', this._updatePosition, this);
    // map.on('move', this._updatePosition, this);
    map.on('resize', this._updateSize, this);
    map.on('zoomend', this._update, this);

    setTimeout(function() {
      app.ports.mapLoaded.send('');
      this._updateSize();
    }.bind(this), 100);
  },

  onRemove: function(map) {
    map.getPanes().overlayPane.removeChild(this._el);
    map.off('dragend', this._updatePosition, this);
    map.off('move', this._updatePosition, this);
    map.off('resize', this._updateSize, this);
    map.off('zoomend', this._update, this);
  },

  _update: function() {
    var bounds = this._map.getPixelBounds().min;
    var size = this._map.getSize();

    app.ports.mapUpdate.send({
      scale: Math.pow(2, this._map.getZoom()),
      zoom: this._map.getZoom(),
      north: bounds.y,
      west: bounds.x,
      width: size.x,
      height: size.y
    });
  },

  _updateSize: function() {
    var size = this._map.getSize();
    this._el.width = size.x;
    this._el.height = size.y;
    this._updatePosition();
  },

  _updatePosition: function() {
    this._update();
    var pos = this._map.containerPointToLayerPoint([0, 0]);
    L.DomUtil.setPosition(this._el, pos);
  }

});

var MapOverlays = {
  _overlays: [],

  setOverlays: function(map, specs) {
    this.clearOverlays();
    this._overlays = specs.map(this._createOverlay);
    this._overlays.forEach(function(overlay) { overlay.addTo(map); });

    var bounds = L.latLngBounds([]);
    this._overlays.forEach(function(overlay) {
      if (overlay.getBounds) {
        bounds.extend(overlay.getBounds());
      }
    });
    map.fitBounds(bounds, {paddingTopLeft: [600, 0]});
  },

  clearOverlays: function() {
    this._overlays.forEach(function(overlay) { overlay.remove(); });
    this._overlays = [];
  },

  _createOverlay: function(spec) {
    var ov;
    if (spec.shape == 'polyline') {
      ov = L.polyline(spec.latlngs, spec.options);
    } else if (spec.shape == 'circleMarker') {
      ov = L.circleMarker(spec.latlngs[0], spec.options);
    }
    if (spec.tooltip) {
      ov = ov.bindTooltip(spec.tooltip, {sticky: true});
    }
    return ov;
  },

};

function initPorts(app) {
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
    c._el = map.getContainer().querySelector('.leaflet-overlay');
    map.addLayer(c);
  });

  app.ports.mapSetOverlays.subscribe(function(m) {
    var map = window.elmMaps[m.mapId];
    MapOverlays.setOverlays(map, m.overlays);
  });

  app.ports.mapClearOverlays.subscribe(function(id) {
    MapOverlays.clearOverlays();
  });

  var ports = {
    'mousedown': app.ports.mapMouseDown,
    'mouseup': app.ports.mapMouseUp,
    'mouseout': app.ports.mapMouseUp,
    'mousemove': app.ports.mapMouseMove
  };

  var model = null;
  document.addEventListener(
      'elmgl-update', function(e) { model = e.detail.newModel; });

  document.addEventListener('elmgl-init', function(e) {
    var canvas = e.detail.canvas;
    var gl = e.detail.gl;
    var render = e.detail.render;

    var sendTelemetry = function(eventType) {
      return function(e) {
        if (!model) {
          console.log('missing model!');
          return;
        }
        render(model, e.clientX, e.clientY, function(color) {
          ports[eventType].send({x: e.clientX, y: e.clientY, color: color});
        });
      };
    };

    var register = function(eventType) {
      canvas.addEventListener(eventType, sendTelemetry(eventType));
    };

    for (var port in ports) {
      register(port);
    }
  });
}
