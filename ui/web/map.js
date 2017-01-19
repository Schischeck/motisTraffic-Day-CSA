var CanvasOverlay = L.Layer.extend({
  initialize: function() { L.setOptions(this, {}); },

  onAdd: function(map) {
    this._map = map;

    map._panes.overlayPane.appendChild(this._el);

    map.on('dragend', this._updatePosition, this);
    map.on('move', this._updatePosition, this);
    map.on('moveend', this._updatePosition, this);
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
    map.off('moveend', this._updatePosition, this);
    map.off('resize', this._updateSize, this);
    map.off('zoomend', this._update, this);
  },

  _update: function() {
    var pixelBounds = this._map.getPixelBounds().min;
    var geoBounds = this._map.getBounds();
    var size = this._map.getSize();
    // var railVizBounds = L.latLngBounds(
    //     this._map.unproject(pixelBounds.subtract(size)),
    //     this._map.unproject(pixelBounds.add(size).add(size)));
    var railVizBounds = geoBounds;

    app.ports.mapUpdate.send({
      scale: Math.pow(2, this._map.getZoom()),
      zoom: this._map.getZoom(),
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
      }
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
    c._el = map.getContainer().querySelector('.railviz-overlay');
    map.addLayer(c);

    RailViz.Render.init(c._el);
  });

  app.ports.mapSetOverlays.subscribe(function(m) {
    var map = window.elmMaps[m.mapId];
    MapOverlays.setOverlays(map, m.overlays);
  });

  app.ports.mapClearOverlays.subscribe(function(id) {
    MapOverlays.clearOverlays();
  });
}
