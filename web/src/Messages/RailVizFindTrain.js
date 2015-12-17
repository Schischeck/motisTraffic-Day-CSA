import Message from './Message';

export default class RailVizRouteAtTimeReq extends Message {
  constructor(trainNumber) {
    super('RailVizRouteAtTimeReq', {
      train_number: trainNumber
    });
  }
}
