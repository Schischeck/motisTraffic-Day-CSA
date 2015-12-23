import Message from './Message';

export default class RailVizRouteAtTimeReq extends Message {
  constructor(routeId, departureTime, departureStation) {
    super('RailVizRouteAtTimeReq', {
      station_id: departureStation,
      departure_time: departureTime,
      route_id: routeId
    });
  }
}
