import Server from './../../Server';
import Dispatcher from './../../Dispatcher';
import RailVizAlltraReq from './../../Messages/RailVizAlltraReq';
import { MapRenderer } from './webgl/MapRenderer';
import { Layer } from './webgl/Layer';
import { Matrix } from './webgl/Matrix';
import Texture from './webgl/Texture';
import TimeStore from './TimeStore';
import RailVizStore from './RailVizStore';
import { Station } from './Station';

export class RailViz {

  constructor(canvas, map) {
    this.stations = null;
    this.map = map;
    this.renderer = new MapRenderer(canvas, map);
    window.requestAnimationFrame(this.updateStations.bind(this));
    this.map.on('move', this.sendRequest.bind(this));
    this.interval = setInterval(this._tick, 1000);
  }
  _tick() {
    Dispatcher.dispatch({
      content_type: 'tick'
    });
  }

  sendRequest() {
    let pp1 = this.map.project(this.map.getBounds().getNorthWest(), 0);
    let pp2 = this.map.project(this.map.getBounds().getSouthEast(), 0);
    let p1 = {
      x: Math.min(pp1.x, pp2.x),
      y: Math.min(pp1.y, pp2.y)
    };
    let p2 = {
      x: Math.max(pp1.x, pp2.x),
      y: Math.max(pp1.y, pp2.y)
    };
    let width = Math.abs(p1.x - p2.x);
    let height = Math.abs(p1.y - p2.y);
    p1.x = p1.x - width;
    p1.y = p1.y - height;
    if (p1.x < 0) p1.x = 0;
    if (p1.y < 0) p1.y = 0;
    p2.x = p2.x + width;
    p2.y = p2.y + height
    let ll1 = this.map.unproject(new L.Point(p1.x, p1.y), 0);
    let ll2 = this.map.unproject(new L.Point(p2.x, p2.y), 0);
    let time = Math.round((TimeStore.getTime() / 1000));
    Server.sendMessage(new RailVizAlltraReq(ll1, ll2, time)).then(response => {
      Dispatcher.dispatch({
        content_type: response.content_type,
        content: response.content
      });
      this.updateTrains();
    });
  }

  updateTrains() {
    let trains = RailVizStore.getTrains();
    if (trains) {
      let drivingTrains = [];
      let tracks = [];
      let metaData = [];
      this.renderer.remove('trains');
      console.log('trains ready');
      let texture = this.renderer.createTextureFromCanvas(Texture.createCircle(20, 
				[71,186,255,255], [59,153,0,255], 8));
      this.trainLayer = new Layer('trains');
      trains.forEach(train => {
        if (train.isDriving(TimeStore.getTime())) {
          drivingTrains.push(train);
          metaData.push({
            start: Math.round(tracks.length),
            size: train.path.length < 2 ? 2 : ((train.path.length - 1) * 2)
          });
          for (let i = 0; i < train.path.length - 1; i++) {
            let station1 = train.path[i];
            let station2 = train.path[i + 1];
            tracks.push(new Station(station1.lat, station1.lng, Math.fround(station1.x), Math.fround(station1.y), station1.name),
						 new Station(station2.lat, station2.lng, Math.fround(station2.x), Math.fround(station2.y), station2.name));
          }
        }
      })
      this.trainLayer.addAll(drivingTrains, texture, {
        zoom: this.map.getZoom()
      });
      this.trainLayer.addAll(tracks, "LINES", metaData);
      this.renderer.add(this.trainLayer);
    }
  }

  updateStations() {
    let s = RailVizStore.getStations();
    if (s) {
      this.stations = s;
      let layer = new Layer('stations');
      console.log('stations ready');
      let texture = this.renderer.createTextureFromCanvas(Texture.createCircle(20, 
				[71,186,0,255], [59,153,0,255], 8));
      layer.addAll(s, texture, null);
      this.renderer.add(layer);
    } else {
      window.requestAnimationFrame(this.updateStations.bind(this));
    }
  }

}
