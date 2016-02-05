window.JAVASCRIPT_MODULES = {};
window.loadJsModules = function(onFinish) {
  function GET(url, cb) {
    var req = new XMLHttpRequest();
    req.onreadystatechange = function() {
      if (req.readyState === 4 &&
          (req.status === 200 || !req.status && req.responseText.length)) {
        cb(req.responseText);
      }
    };
    req.open('GET', url, true);
    req.send('');
  }

  function globalEval(js) {
    var node = document.createElement('script');
    var root = document.documentElement;
    node.text = js;
    root.appendChild(node);
    root.removeChild(node);
  }

  function compile(url, src) {
    var code = compileJsModule(src, {}, url);

    if (url) {
      code += '\n//# sourceURL=' + url;
    }
    globalEval(code);
    if (!--numScriptsToCompile) {
      onFinish();
    }
  }

  function allReceived() {
    if (scriptSources.length !== scripts.length) {
      return false;
    }

    for (var i = 0; i < scriptSources.length; i++) {
      if (typeof scriptSources[i] !== 'object') {
        return false;
      }
    }

    return true;
  }

  function receive(index, url, src) {
    scriptSources[index] = {'url': url, 'src': src};
    if (allReceived()) {
      scriptSources.forEach(el => compile(el.url, el.src));
    }
  }

  var scripts = document.querySelectorAll('script[type="module"]');
  var scriptSources = [];
  var numScriptsToCompile = scripts.length;

  if (numScriptsToCompile === 0) {
    onFinish();
  } else {
    for (var i = 0; i < scripts.length; i++) {
      var script = scripts[i];
      var url = script.getAttribute('src');
      if (url) {
        GET(url, receive.bind(null, i, url));
      } else {
        receive(null, i, script.innerHTML);
      }
    }
  }
}
