import { ReduceStore } from 'flux/utils';
import Immutable from 'immutable';

import AppDispatcher from './Dispatcher';

class ConnectionStateStore extends ReduceStore {
  constructor(dispatcher) {
    super(dispatcher);
  }

  getInitialState() {
    return new Immutable.Map({
      'connected': false
    });
  }

  isConnected() {
    return this.getState().get('connected');
  }

  reduce(state, action) {
    switch (action.type) {
      case 'ConnectionStateChange':
        return state.set('connected', action.connectionState);
      default:
        return state;
    }
  }
}

const instance = new ConnectionStateStore(AppDispatcher);
export default instance;
