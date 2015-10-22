import Dispatcher from './Dispatcher';
import Server from '../Server';

const Actions = {
  sendMessage: function(message, componentId) {
    Server.sendMessage(message).then((res) => {
      Dispatcher.dispatch({
        type: res.content_type,
        content: res.content,
        componentId: componentId
      });
    });
  }
}

export default Actions;

