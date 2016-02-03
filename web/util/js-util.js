window.modulo = function(a, n) { return ((a % n) + n) % n; }

window.safeCall = function(fun, ...args) {
  if (fun) {
    return fun(...args);
  }
}

window.entries = function(obj) {
  return Object.keys(obj).map(key => {
    return { key, value: colors[key] };
  });
}

window.not = function(fn) {
  return function() {
    return !fn(arguments);
  }
}

window.padZeros = function(num, size) {
  const s = '00000' + num;
  return s.substr(s.length - size);
}

window.formatTime = function(t) {
  return padZeros(t.getHours(), 2) + ':' + padZeros(t.getMinutes(), 2);
}