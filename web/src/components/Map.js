import React from 'react';

import L from 'leaflet';

import './Map.scss';
import 'leaflet/dist/leaflet.css';

export default class Map extends React.Component {
  constructor(props) {
    super(props);
  }

  componentDidMount() {
    this.map = L.map('map', {
      'zoomControl': false
    });
    this.map.setView([
      51.505,
      -0.09
    ], 13);http://
    //L.tileLayer('http://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    //L.tileLayer('http://c.tile.stamen.com/watercolor/{z}/{x}/{y}.jpg', {
    L.tileLayer('http://a.basemaps.cartocdn.com/dark_all/{z}/{x}/{y}.png', {
      attribution: '&copy; OpenStreetMap-Mitwirkende'
    }).addTo(this.map);
  }

  render() {
    return (
    <div id="map"></div>
    );
  }
}
