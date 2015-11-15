class Server {
  constructor(server) {
    this.requestId = 0;
    this.socket = new WebSocket(server);
    this.socket.onmessage = this._onmessage.bind(this);
    this.socket.onclose = () => {
      console.log('close', arguments);
    };
    this.socket.onerror = () => {
      console.log('error', arguments);
    };
    this.socket.onopen = () => {
      console.log('open', arguments);
    };

    // TODO should be a Map not Object type
    this.pendingRequests = {};
  }

  _onmessage(evt) {
    console.log(JSON.parse(evt.data));
    this._resolvePending(JSON.parse(evt.data));
  }

  _isPendingRequest(id) {
    return this.pendingRequests[id] !== undefined;
  }

  _cancelTimeout(id) {
    clearTimeout(this.pendingRequests[id].timer);
  }

  _resolvePending(data) {
    if (!this._isPendingRequest(data.id)) {
      return;
    }

    this._cancelTimeout(data.id);
    if (data.content_type === 'MotisError') {
      this.pendingRequests[data.id].reject(data);
    } else {
      this.pendingRequests[data.id].resolve(data);
    }
    delete this.pendingRequests[data.id];
  }

  _rejectPending(id, reason) {
    if (!this._isPendingRequest(id)) {
      return;
    }

    this._cancelTimeout(id);
    this.pendingRequests[id].reject(reason);
    delete this.pendingRequests[id];
  }

  sendMessage(message) {
    return new Promise((resolve, reject) => {
      const localRequestId = ++this.requestId;
      const request = {
        'id': localRequestId,
        'content_type': message.contentType,
        'content': message.content
      };

      this.socket.send(JSON.stringify(request));

      const timer = setTimeout(() => {
        this._rejectPending(localRequestId, 'timeout');
      }, message.timeout);

      this.pendingRequests[localRequestId] = {
        resolve: resolve,
        reject: reject,
        timer: timer
      };
    });
  }
}

const instance = new Server('ws://localhost:8080');
export default instance;
