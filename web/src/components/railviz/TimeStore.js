import { ReduceStore, Container } from 'flux/utils';
import Immutable from 'immutable';

import AppDispatcher from '../../Dispatcher';

class TimeStore extends ReduceStore {
  getInitialState() {
    let time = new Date().getTime();
    return Immutable.Map({
      simulatedTime: time
    });
  }

  reduce(state, action) {
    switch (action.content_type) {
      case 'RailVizInit':
        return state.set('schedule_end', action.content['schedule_end'] * 1000);
      case 'tick':
        let simTime = state.get('simulatedTime');
        return state.set('simulatedTime', simTime + 1000);
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
