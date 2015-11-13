import { ReduceStore } from 'flux/utils';
import Immutable from 'immutable';

import AppDispatcher from './Dispatcher';

class Store extends ReduceStore {
  getInitialState() {
    return new Immutable.Map({
      value: 0
    });
  }

  reduce(state, action) {
    switch (action.type) {
      case 'StationGuesserResponse':
        return state.set(action.componentId, action.content);
      default:
        return state;
    }
  }
}

const instance = new Store(AppDispatcher);
export default instance;
