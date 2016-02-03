Server = function() {
  const params = location.search.slice(1).split('&').reduce((map, el) => {
    if (el === '') {
      return map;
    }
    const keyval = el.split('=', 2);
    return map.set(keyval[0], keyval[1] || true);
  }, new Map());
  const host = params.get('host') || window.location.hostname || '127.0.0.1';
  const port = params.get('port') || '8080';
  const path = params.get('path') || '/';
  const url = 'ws://' + host + ':' + port + path;

  riot.observable(this);
  this.url = url;
  this.pendingRequests = new Map();
  this.nextRequestId = 1;
  this.connect();
}

Server.prototype.isConnected = function() {
  return this.socket.readyState === 1;
}

Server.prototype.connect = function() {
  this.socket = new WebSocket(this.url);
  this.socket.onmessage = this.onMessage.bind(this);
  this.socket.onopen = () => {
    this.trigger('connected', true);
  };
  this.socket.onclose = () => {
    this.trigger('connected', false);
    setTimeout(() => this.connect(), 2000);
  };
}

Server.prototype.onMessage = function(ev) {
  const data = JSON.parse(ev.data);

  if (data.id === 0) {
    this.trigger('msg', data);
    return;
  }

  if (!this.pendingRequests.has(data.id)) {
    if (data.content_type === 'MotisError') {
      this.trigger('error', data);
    }
    return;
  }

  var pending = this.pendingRequests.get(data.id);
  this.cancelTimeout(data.id);
  this.pendingRequests.delete(data.id);
  if (data.content_type === 'MotisError') {
    pending.reject(data);
  } else {
    pending.resolve(data);
  }
}

Server.prototype.cancelTimeout = function(id) {
  clearTimeout(this.pendingRequests.get(id).timer);
}

Server.prototype.send = function(request, timeout) {
  return new Promise((resolve, reject) => {
    request.id = this.nextRequestId++

    try {
      this.socket.send(JSON.stringify(request));
    } catch (e) {
      reject('disconnected');
    }

    const timer = setTimeout(() => {
      this.cancelTimeout(request.id);
      this.pendingRequests.get(request.id).reject('timeout');
      this.pendingRequests.delete(request.id);
    }, timeout || 60000);

    this.pendingRequests.set(request.id, {
      resolve: resolve,
      reject: reject,
      timer: timer
    });
  });
}

export default new Server;