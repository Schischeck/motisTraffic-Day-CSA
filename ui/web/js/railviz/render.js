var RailViz = RailViz || {};

RailViz.Render = (function() {

  const n_stations = 100;
  const n_routes = 200;
  const n_trains = 1000;

  var data;
  var mapInfo;
  var timeOffset = 0;
  var canvas;
  var gl;
  var startTime;
  var rafRequest;
  var offscreen = {};
  var mouseHandler;
  const pixelRatio = window.devicePixelRatio;

  function init(c, mouseEventHandler) {
    // data = RailViz.DemoGenerator.generate(
    //     n_stations, n_routes, n_trains, 1600, 860);
    // RailViz.Preprocessing.preprocess(data);
    setData({stations: [], routes: [], trains: []});
    setMapInfo({zoom: 14, scale: 1, pixelBounds: {north: 0, west: 0}});

    canvas = c;
    mouseHandler = mouseEventHandler || function() {};

    // https://www.khronos.org/webgl/wiki/HandlingContextLost
    canvas.addEventListener('webglcontextlost', contextLost, false);
    canvas.addEventListener('webglcontextrestored', contextRestored, false);

    const mouseEvents = ['mousedown', 'mouseup', 'mousemove', 'mouseout'];

    mouseEvents.forEach(
        eventType => canvas.addEventListener(
            eventType, event => handleMouseEvent(eventType, event)));

    setup();
  }

  function setData(newData) {
    data = newData;

    RailViz.Stations.init(data.stations);
    RailViz.Routes.init(data.routes);
    RailViz.Trains.init(data.trains, data.routes);
  }

  function setMapInfo(newMapInfo) { mapInfo = newMapInfo; }

  function setTimeOffset(newTimeOffset) { timeOffset = newTimeOffset; }

  function setup() {
    gl = canvas.getContext('webgl') || canvas.getContext('experimental-webgl');
    if (!gl) {
      console.log('Could not initialize WebGL!');
      return;
    }

    offscreen = {};

    RailViz.Stations.setup(gl);
    RailViz.Routes.setup(gl);
    RailViz.Trains.setup(gl);

    startTime = performance.now();

    rafRequest = requestAnimationFrame(render);
  }

  function render(timestamp) {
    WebGL.Util.resizeCanvasToDisplaySize(canvas, pixelRatio);

    var perspective = WebGL.Util.makeOrtho2D(
        0, gl.canvas.clientWidth, gl.canvas.clientHeight, 0);
    WebGL.Util.m4Scale(
        perspective, [mapInfo.scale, mapInfo.scale, mapInfo.scale]);
    WebGL.Util.m4Translate(perspective, [
      -(mapInfo.pixelBounds.west / mapInfo.scale),
      -(mapInfo.pixelBounds.north / mapInfo.scale), 0
    ]);
    var zoom = mapInfo.zoom * pixelRatio;

    var time = timeOffset + (Date.now() / 1000);

    createOffscreenBuffer();

    RailViz.Trains.preRender(gl, time);

    for (var i = 0; i <= 1; i++) {
      var isOffscreen = i == 0;

      gl.bindFramebuffer(
          gl.FRAMEBUFFER, isOffscreen ? offscreen.framebuffer : null);
      gl.viewport(0, 0, gl.canvas.width, gl.canvas.height);
      gl.enable(gl.BLEND);
      gl.disable(gl.DEPTH_TEST);
      gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);
      gl.clearColor(0, 0, 0, 0);
      gl.clear(gl.COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT);

      RailViz.Routes.render(gl, perspective, isOffscreen);
      RailViz.Stations.render(gl, perspective, zoom, isOffscreen);
      RailViz.Trains.render(gl, perspective, zoom, isOffscreen);
    }

    rafRequest = requestAnimationFrame(render);
  }

  function createOffscreenBuffer() {
    var width = gl.drawingBufferWidth;
    var height = gl.drawingBufferHeight;

    if (offscreen.width === width && offscreen.height === height &&
        offscreen.framebuffer) {
      return;
    }

    offscreen.width = width;
    offscreen.height = height;

    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
    if (offscreen.framebuffer && gl.isFramebuffer(offscreen.framebuffer)) {
      gl.deleteFramebuffer(offscreen.framebuffer);
      offscreen.framebuffer = null;
    }
    if (offscreen.renderbuffer && gl.isRenderbuffer(offscreen.renderbuffer)) {
      gl.deleteRenderbuffer(offscreen.renderbuffer);
      offscreen.renderbuffer = null;
    }
    if (offscreen.texture && gl.isTexture(offscreen.texture)) {
      gl.deleteTexture(offscreen.texture);
      offscreen.texture = null;
    }

    offscreen.texture = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, offscreen.texture);
    gl.texImage2D(
        gl.TEXTURE_2D, 0, gl.RGBA, width, height, 0, gl.RGBA, gl.UNSIGNED_BYTE,
        null);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);

    // offscreen.renderbuffer = gl.createRenderbuffer();
    // gl.bindRenderbuffer(gl.RENDERBUFFER, offscreen.renderbuffer);
    // gl.renderbufferStorage(gl.RENDERBUFFER, gl.DEPTH_COMPONENT16, width,
    // height);

    offscreen.framebuffer = gl.createFramebuffer();
    gl.bindFramebuffer(gl.FRAMEBUFFER, offscreen.framebuffer);
    gl.framebufferTexture2D(
        gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, offscreen.texture,
        0);
    // gl.framebufferRenderbuffer(gl.FRAMEBUFFER, gl.DEPTH_ATTACHMENT,
    // gl.RENDERBUFFER, offscreen.renderbuffer);

    gl.bindTexture(gl.TEXTURE_2D, null);
    gl.bindRenderbuffer(gl.RENDERBUFFER, null);
    gl.bindFramebuffer(gl.FRAMEBUFFER, null);
  }

  function readOffscreenPixel(x, y) {
    if (!offscreen.framebuffer || !gl.isFramebuffer(offscreen.framebuffer)) {
      return null;
    }

    var pixels = new Uint8Array(4);
    gl.bindFramebuffer(gl.FRAMEBUFFER, offscreen.framebuffer);
    gl.readPixels(
        x, gl.drawingBufferHeight - y, 1, 1, gl.RGBA, gl.UNSIGNED_BYTE, pixels);

    return pixels;
  }

  function contextLost(event) {
    event.preventDefault();
    cancelAnimationFrame(rafRequest);
    console.log('WebGL context lost');
  }

  function contextRestored(event) {
    console.log('WebGL context restored');
    setup();
  }

  function handleMouseEvent(eventType, event) {
    const mouseX = (event.pageX - canvas.offsetLeft);
    const mouseY = (event.pageY - canvas.offsetTop);
    const pickingX = mouseX * pixelRatio;
    const pickingY = mouseY * pixelRatio;

    const offscreenPixel = readOffscreenPixel(pickingX, pickingY);
    const pickId = RailViz.Picking.colorToPickId(offscreenPixel);

    const pickedTrainIndex = RailViz.Trains.getPickedTrainIndex(pickId);
    const pickedStationIndex = RailViz.Stations.getPickedStationIndex(pickId);

    const pickedTrain =
        pickedTrainIndex != null ? data.trains[pickedTrainIndex] : null;
    const pickedStation =
        pickedStationIndex != null ? data.stations[pickedStationIndex] : null;

    if (pickId && eventType != 'mouseout') {
      canvas.style.cursor = 'pointer';
    } else {
      canvas.style.cursor = 'default';
    }

    mouseHandler(eventType, mouseX, mouseY, pickedTrain, pickedStation);
  }

  return {
    init: init,
    setData: setData,
    setMapInfo: setMapInfo,
    setTimeOffset: setTimeOffset,
    render: render,
    getCanvas: function() { return canvas; }
  };

})();
