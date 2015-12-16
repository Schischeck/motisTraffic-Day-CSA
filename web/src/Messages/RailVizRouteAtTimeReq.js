import Message from './Message';

export default class RailVizRouteAtTimeReq extends Message {
  constructor(routeId, departure_time, departure_station) {
    super('RailVizRouteAtTimeReq', {
				station_id: departure_station,
		    departure_time: departure_time,
		    route_id: routeId
			});
  }
}