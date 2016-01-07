import { ReduceStore, Container } from 'flux/utils';
import Immutable from 'immutable';

import AppDispatcher from '../../Dispatcher';

class TimeStore extends ReduceStore {
  getInitialState() {
    let time = Date.now();
    return Immutable.Map({
      simulatedTime: time,
			lastnow: 0
    });
  }

  reduce(state, action) {
    switch (action.content_type) {
      case 'RailVizInit':
				state.set('lastnow', performance.now());
        return state.set('schedule_end', action.content['schedule_end'] * 1000);
      case 'tick':
        let simTime = state.get('simulatedTime');
        return state.set('simulatedTime', simTime + 10);
      default:
        return state;
    }
  }

  getTime() {
    return this.getState().get('simulatedTime');
  }

  getscheduleStart() {
    return this.getState().get('schedule_start');
  }

}

const instance = new TimeStore(AppDispatcher);
export default instance;
