import Message from './Message';

export default class RoutingRequest extends Message {
  constructor(begin, path, intervalLength = 3000, type = 'PreTrip', direction = 'Forward') {
    super('RoutingRequest', {
      'interval': {
        'begin': begin,
        'end': begin + intervalLength
      },
      'type': type,
      'direction': direction,
      'path': path.map(station => {
        return {
          'eva_nr': station,
          'name': ''
        };
      })
    }, 30000);
    console.log(this);
  }
}
