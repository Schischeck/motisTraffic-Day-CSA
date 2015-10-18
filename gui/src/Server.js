import Actions from './flux-infra/Actions';

class Server {
  constructor(server) {
    this.requestId = 0;
    this.socket = new WebSocket(server);
    this.socket.onmessage = this._onmessage.bind(this);
    this.socket.onclose = () => {console.log('close', arguments)};
    this.socket.onerror = () => {console.log('error', arguments)};
    this.socket.onopen = () => {console.log('open', arguments)};

    this.pendingRequests = {};
  }

  _onmessage(evt) {
    let res = JSON.parse(evt.data);

    let hasId = true;
    if (res.id === undefined) {
      console.error('received message without id');
      hasId = false;
    }

    switch (res.content_type) {
      case 'StationGuesserResponse':
        this._resolvePending(res.id, res.content);
        break;
      default:
        this._rejectPending(res.id, 'unknown message type');
    }
  };

  _isPendingRequest(id) {
     return this.pendingRequests[id] != undefined;
  }

  _cancelTimeout(id) {
    clearTimeout(this.pendingRequests[id].timer);
  }

  _resolvePending(id, data) {
    if (!this._isPendingRequest(id)) {
      return;
    }

    this._cancelTimeout(id);
    this.pendingRequests[id].resolve(data);
    delete this.pendingRequests[id];
  }

  _rejectPending(id, reason) {
    if (!this._isPendingRequest(id)) {
      return;
    }

    this._cancelTimeout(id);
    this.pendingRequests[id].reject(reason);
    delete this.pendingRequests[id];
  }

  getStationSuggestions(inputValue, inputName) {
    return new Promise((resolve, reject) => {
      let localRequestId = ++this.requestId;
      let request = {
        'id': localRequestId,
        'content_type': 'StationGuesserRequest',
        'content': {
          'input': inputValue,
          'guess_count': 5
        }
      };

      this.socket.send(JSON.stringify(request));

      let timer = setTimeout(() => {
        reject('timeout');
        delete this.pendingRequests[localRequestId];
      }, 1000);

      this.pendingRequests[localRequestId] = {
        resolve: resolve,
        reject: reject,
        timer: timer
      };
    });
  }
};

const instance = new Server('ws://localhost:8080');
export default instance;
