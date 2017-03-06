
function initApp() {
  var params = getQueryParameters();

  var defaultHost = window.location.hostname;
  var defaultPort = '8080';
  var apiEndpoint = 'http://' + defaultHost + ':' + defaultPort + '/';
  var motisParam = params['motis'] || null;
  if (motisParam) {
    if (/^[0-9]+$/.test(motisParam)) {
      apiEndpoint = 'http://' + defaultHost + ':' + motisParam + '/';
    } else if (!motisParam.includes(':')) {
      apiEndpoint = 'http://' + motisParam + ':' + defaultPort + '/';
    } else if (!motisParam.startsWith('http:')) {
      apiEndpoint = 'http://' + motisParam;
      if (!apiEndpoint.endsWith('/')) {
        apiEndpoint += '/';
      }
    } else {
      apiEndpoint = motisParam;
    }
  }

  var simulationTime = null;
  var timeParam = params['time'] || null;
  if (timeParam) {
    simulationTime = parseTimestamp(timeParam);
  }

  var langParam = params['lang'] || null;
  var language = langParam || 'de';

  window.app = Elm.Main.embed(document.getElementById('app-container'), {
    apiEndpoint: apiEndpoint,
    currentTime: Date.now(),
    simulationTime: simulationTime,
    language: language,
    motisParam: motisParam,
    timeParam: timeParam,
    langParam: langParam,
    fromLocation: localStorage.getItem('motis.routing.from_location'),
    toLocation: localStorage.getItem('motis.routing.to_location')
  });

  window.elmMaps = {};

  initPorts(app, apiEndpoint);
  handleDrop(document.getElementById('app-container'));
  app.ports.localStorageSet.subscribe(function(kv) {
    localStorageSet(kv[0], kv[1]);
  });
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

function localStorageSet(key, value) {
  try {
    localStorage.setItem(key, value);
  } catch (ex) {
  }
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
