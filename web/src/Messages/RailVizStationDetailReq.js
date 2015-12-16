import Message from './Message';

export default class RailVizStationDetailReq extends Message {
  constructor(id) {
    super('RailVizStationDetailReq', {
			station_index: id
			});
  }
}