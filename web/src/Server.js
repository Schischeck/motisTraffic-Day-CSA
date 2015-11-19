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

    this.pendingRequests = new Map();
  }

  _onmessage(evt) {
    const msg = evt.data.replace('\\x', '\\u00');
    try {
      this._resolvePending(JSON.parse(msg));
    } catch (e) {
      console.error('invalid json', msg);
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
