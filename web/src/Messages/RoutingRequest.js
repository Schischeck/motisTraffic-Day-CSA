import Message from './Message';

export default class RoutingRequest extends Message {
  constructor(/* begin, end, type, direction, path */) {
    super('RoutingRequest', {
        "interval": {
          "begin": 1444896228,
          "end": 1444899228
        },
        "type": "PreTrip",
        "direction": "Forward",
        "path": [
          { "eva_nr": "8000096", "name": "" },
          { "eva_nr": "8000105", "name": "" }
        ]
    }, 20000);
  }
}
