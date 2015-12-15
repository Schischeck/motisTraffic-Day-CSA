import AppDispatcher from './Dispatcher';

class Server {
  constructor(server) {
    this.requestId = 0;
    this.server = server;
    this.pendingRequests = new Map();

    this._wsConnect();
  }

  _wsConnect() {
    this.socket = new WebSocket(this.server);
    this.socket.onmessage = this._onMessage.bind(this);
    this.socket.onopen = () => {
      this.sendAuth().then(response => {
        console.log('auth response', response);
        AppDispatcher.dispatch({
          'type': 'ConnectionStateChange',
          'connectionState': true
        });
      });
    };
    this.socket.onclose = () => {
      AppDispatcher.dispatch({
        'type': 'ConnectionStateChange',
        'connectionState': false
      });
      setTimeout(() => {
        this._wsConnect();
      }, 2000);
    };
  }

  _onMessage(evt) {
    try {
      this._resolvePending(JSON.parse(evt.data));
    } catch (e) {
      console.error('invalid json', evt.data);
    }
  }

  _isPendingRequest(id) {
    return this.pendingRequests.has(id);
  }

  _cancelTimeout(id) {
    clearTimeout(this.pendingRequests.get(id).timer);
  }

  _resolvePending(data) {
    if (!this._isPendingRequest(data.id)) {
			AppDispatcher.dispatch(data);
      return;
    }

    this._cancelTimeout(data.id);
    if (data.content_type === 'MotisError') {
      this.pendingRequests.get(data.id).reject(data);
    } else {
      this.pendingRequests.get(data.id).resolve(data);
    }
    this.pendingRequests.delete(data.id);
  }

  _rejectPending(id, reason) {
    if (!this._isPendingRequest(id)) {
      return;
    }

    this._cancelTimeout(id);
    this.pendingRequests.get(id).reject(reason);
    this.pendingRequests.delete(id);
  }

  sendAuth() {
    return new Promise((resolve, reject) => {
      console.log('API KEY: ', __MOTIS_API_KEY__);
      if (__MOTIS_API_KEY__ == undefined || !__MOTIS_API_KEY__) {
        console.log('immediate resolve');
        return resolve({ 'id': 0 });
      }

      try {
        this.socket.send(__MOTIS_API_KEY__);
      } catch (e) {
        reject(e);
      }

      const timer = setTimeout(() => {
        this._rejectPending(0, 'timeout');
      }, 1000);

      this.pendingRequests.set(0, {
        resolve: resolve,
        reject: reject,
        timer: timer
      });
    });
  }

  sendMessage(message) {
    return new Promise((resolve, reject) => {
      const localRequestId = ++this.requestId;
      const request = {
        'id': localRequestId,
        'content_type': message.contentType,
        'content': message.content
      };

      try {
        this.socket.send(JSON.stringify(request));
      } catch (e) {
        reject(e);
      }

      const timer = setTimeout(() => {
        this._rejectPending(localRequestId, 'timeout');
      }, message.timeout);

      this.pendingRequests.set(localRequestId, {
        resolve: resolve,
        reject: reject,
        timer: timer
      });
    });
  }
}


function parseURL() {
  return location.search.slice(1).split('&').reduce((map, el) => {
    if (el === '') return map;
    const keyval = el.split('=', 2);
    return map.set(keyval[0], keyval[1] || true);
  }, new Map());
};
const params = parseURL();
const host = params.get('host') || __MOTIS_REMOTE_HOST__ || window.location.hostname;
const port = params.get('port') || __MOTIS_REMOTE_PORT__ || '8080';
const path = params.get('path') || __MOTIS_REMOTE_PATH__ || '/';
const url = 'ws://' + host + ':' + port + path;

const instance = new Server(url);
export default instance;
