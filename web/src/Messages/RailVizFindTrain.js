import Message from './Message';

export default class RailVizRouteAtTimeReq extends Message {
  constructor(train_number) {
    super('RailVizRouteAtTimeReq', {
				train_number: train_number
			});
  }
}