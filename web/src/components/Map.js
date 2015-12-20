import React from 'react';
import L from 'leaflet';
import { RailViz } from './railviz/RailViz';
import Dispatcher from '../Dispatcher';

import './Map.scss';
import 'leaflet/dist/leaflet.css';

export default class Map extends React.Component {
  constructor(props) {
    super(props);
  }

  componentDidMount() {
    this.map = L.map('map', {
      'zoomControl': false,
      'zoomAnimation': false,
      'inertia': false
    });
    this.map.setView([
      49.87169,
      8.65291
    ], 13);//
    // L.tileLayer('http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    // L.tileLayer('http://c.tile.stamen.com/watercolor/{z}/{x}/{y}.jpg', {
    L.tileLayer('http://a.basemaps.cartocdn.com/light_all/{z}/{x}/{y}.png', {
      attribution: '&copy; OpenStreetMap-Mitwirkende'
    }).addTo(this.map);
    this.layer = new RailViz(this.refs.canvas, this.map);
    this._initMap(this.map);
  }

  _initMap() {
    Dispatcher.dispatch({
      content_type: 'MapInit',
      content: {
        'map': this.map
      }
    });
  }

  handleResize(/* e */) {
    this.layer.onResize(this.refs.canvas);
  }

  render() {
    return (
    <div id="map">
      <canvas
              ref="canvas"
              id="glCanvas"
              height={ window.innerHeight }
              width={ window.innerWidth }></canvas>
    </div>
    );
  }
}
