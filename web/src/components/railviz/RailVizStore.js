import { ReduceStore, Container } from 'flux/utils';
import Immutable from 'immutable';
import AppDispatcher from '../../Dispatcher';
import { Train } from './Train';
import { Station } from './Station';

class RailVizStore extends ReduceStore {
  getInitialState() {
    return Immutable.Map();
  }

  reduce(state, action) {
    switch (action.content_type) {
      case 'MapInit':
        return state.set('map', action.content['map']);
      case 'RailVizInit':
        let s = this._convertToStations(action.content['stations']);
        return state.set('stations', s);
      case 'RailVizAlltraRes':
        return state.set('trains', this._convertToTrains(action.content['trains'], action.content['routes']));
      default:
        return state;
    }
  }

  getStations() {
    return this.getState().get('stations');
  }

  getTrains() {
    return this.getState().get('trains');
  }

  getMap() {
    return this.getState().get('map');
  }

  _convertToStations(s) {
    let stations = [];
		let index = 0;
    s.forEach(station => {
      let ll = new L.LatLng(station.station_coord.lat, station.station_coord.lng);
      let point = this.getMap().project(ll, 0);
      let newStation = new Station(index, station.station_coord.lat, station.station_coord.lng, point.x, point.y, station.station_name);
      stations.push(newStation);
			index++;
    });
    return stations;
  }

  _convertToTrains(trains, routes) {
    let trainObjects = [];
    let stations = this.getStations();
    let index = 0;
    trains.forEach(train => {
      let startStation = stations[train.d_station];
      let endStation = stations[train.a_station];
      let startDelay = 0;
      let endDelay = 0;
      if (train.d_time_delay) {
        startDelay = train.d_time_delay;
      }
      if (train.a_time_delay) {
        endDelay = train.a_time_delay;
      }
      let route = [];
      if (routes[index]) {
        let route_ids = routes[index].route;
        for (let j = 0; j < route_ids.length; j++) {
          route.push(stations[route_ids[j]]);
        }
      }
      let t = new Train(train.d_time * 1000,
        train.a_time * 1000,
        startStation,
        endStation,
        train.route_id,
        route,
        startDelay,
        endDelay);
      trainObjects.push(t);
      index++;
    });
    return trainObjects;
  }
}

const instance = new RailVizStore(AppDispatcher);
export default instance;
