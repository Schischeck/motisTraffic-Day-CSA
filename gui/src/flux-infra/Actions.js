import AppDispatcher from './Dispatcher';

const Actions = {
  up: function() {
    AppDispatcher.dispatch({
      type: 'up'
    });
  },

  down: function() {
    AppDispatcher.dispatch({
      type: 'down'
    });
  }
}

export default Actions;

