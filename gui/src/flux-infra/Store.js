import {ReduceStore, Container} from 'flux/utils';
import Immutable from 'immutable';

import AppDispatcher from './Dispatcher';

class CounterStore extends ReduceStore {
  getInitialState() {
    return Immutable.Map({value: 0});
  }

  reduce(state, action) {
    switch(action.type) {
      case 'up':
        return state.set('value', state.get('value') + 1);
      case 'down':
        return state.set('value', state.get('value') - 1);
      default:
        return state;
    }
  }
}

const instance = new CounterStore(AppDispatcher);
export default instance;
