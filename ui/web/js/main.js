
function initApp() {
  var params = getQueryParameters();

  var defaultHost = window.location.hostname;
  var defaultPort = '8080';
  var apiEndpoint = 'http://' + defaultHost + ':' + defaultPort + '/';
  if (params['motis']) {
    var p = params['motis'];
    if (/^[0-9]+$/.test(p)) {
      apiEndpoint = 'http://' + defaultHost + ':' + p + '/';
    } else if (!p.includes(':')) {
      apiEndpoint = 'http://' + p + ':' + defaultPort + '/';
    } else if (!p.startsWith('http:')) {
      apiEndpoint = 'http://' + p;
      if (!apiEndpoint.endsWith('/')) {
        apiEndpoint += '/';
      }
    } else {
      apiEndpoint = p;
    }
  }

  var simulationTime = null;
  if (params['time']) {
    simulationTime = parseTimestamp(params['time']);
  }

  var language = params['lang'] || params['lng'] || 'de';

  window.app = Elm.Main.embed(document.getElementById('app-container'), {
    apiEndpoint: apiEndpoint,
    currentTime: Date.now(),
    simulationTime: simulationTime,
    language: language
  });

  window.elmMaps = {};

  initPorts(app, apiEndpoint);
  handleDrop(document.getElementById('app-container'));
}

window.addEventListener('load', initApp);

function handleDrop(element) {
  element.addEventListener('drop', function(e) {
    e.preventDefault();
    var files = e.dataTransfer.files;
    if (files.length == 0) {
      return;
    }
    var contents = [];
    var remaining = files.length;

    function onLoad(i) {
      return function(ev) {
        contents[i] = ev.target.result;
        if (--remaining == 0) {
          var data = [];
          for (var j = 0; j < files.length; j++) {
            data[j] = [files[j].name, contents[j]];
          }
          app.ports.setRoutingResponses.send(data);
        }
      }
    }

    for (var i = 0; i < files.length; i++) {
      var reader = new FileReader();
      reader.addEventListener('load', onLoad(i));
      reader.readAsText(files[i]);
    }
  });
  element.addEventListener('dragenter', function(e) {
    e.preventDefault();
  });
  element.addEventListener('dragover', function(e) {
    e.preventDefault();
  });
}

function getQueryParameters() {
  var params = {};
  window.location.search.substr(1).split('&').forEach(p => {
    var param = p.split('=');
    params[param[0]] = decodeURIComponent(param[1]);
  });
  return params;
}

function parseTimestamp(value) {
  var filterInt = function(value) {
    if (/^(\-|\+)?([0-9]+|Infinity)$/.test(value)) return Number(value);
    return NaN;
  };
  if (value != null) {
    var time = filterInt(value);
    if (time) {
      return time * 1000;
    } else {
      var date = new Date(value);
      var time = date.getTime();
      return (time && !isNaN(time)) ? time : null;
    }
  }
  return null;
}
