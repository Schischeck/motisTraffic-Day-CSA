import Message from './Message';

export default class RailVizAlltraReq extends Message {
  constructor(p1, p2, time) {
    super('RailVizAlltraReq', {
				'p1': p1,
				'p2': p2,
				'time': time,
				'with_routes': true
			});
  }
}