class Server {
  constructor(server) {
    this.requestId = 0;
    this.server = server;
    this.pendingRequests = new Map();

    this._wsConnect();
    setInterval(this._wsCheck.bind(this), 5000);
  }

  _wsConnect() {
    this.socket = new WebSocket(this.server);
    this.socket.onmessage = this._onMessage.bind(this);
    this.socket.onopen = () => {
      console.log('connected');
    };
    this.socket.onclose = () => {
      console.log('disconnected');
    };
  }

  _wsCheck() {
    if (!this.socket || this.socket.readyState === 3) {
      console.log('reconnect');
      this._wsConnect();
    }
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

      this.pendingRequests.set(localRequestId, {
        resolve: resolve,
        reject: reject,
        timer: timer
      });
    });
  }
}

const instance = new Server('ws://localhost:8080');
export default instance;
