import AppDispatcher from './Dispatcher';
import Server from '../Server';

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
  },

  getStationSuggestions: function(data, componentId) {
    Server.getStationSuggestions(data, componentId).then((res) => {
      AppDispatcher.dispatch({
        type: 'guesserResponse',
        data: res,
        componentId: componentId
      });
    });
  }
}

export default Actions;

