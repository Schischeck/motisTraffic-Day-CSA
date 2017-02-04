var RailViz = RailViz || {};

RailViz.Stations = (function() {
  const vertexShader = `
        attribute vec2 a_pos;
        attribute vec4 a_pickColor;
        attribute float a_flags;
        
        uniform mat4 u_perspective;
        uniform float u_zoom;
        uniform bool u_highlightStations;
        
        varying vec4 v_pickColor;
        
        void main() {
            float flags = a_flags;
            gl_Position = u_perspective * vec4(a_pos, 0.0, 1.0);
            float size = u_zoom - 4.0;
            if (u_highlightStations) {
              if (flags >= 128.0) {
                size = u_zoom - 2.0;
              } else {
                size = u_zoom - 8.0;
              }
            }
            gl_PointSize = size;
            v_pickColor = a_pickColor;
        }
    `;

  const fragmentShader = `
        precision mediump float;
        
        uniform bool u_offscreen;
        uniform sampler2D u_texture;
        
        varying vec4 v_pickColor;

        const vec4 transparent = vec4(0.0, 0.0, 0.0, 0.0);
        
        void main() {
            vec4 tex = texture2D(u_texture, gl_PointCoord);
            if (u_offscreen) {
                gl_FragColor = tex.a == 0.0 ? transparent : v_pickColor;
            } else {
                gl_FragColor = tex;
            }
        }
    `;

  var stations = [];
  var highlightedStations;
  var buffer = null;
  var bufferValid = false;
  var texture = null;
  var program;
  var a_pos;
  var a_pickColor;
  var a_flags;
  var u_perspective;
  var u_zoom;
  var u_offscreen;
  var u_texture;
  var u_highlightStations;

  const PICKING_BASE = 100000;

  function init(newStations) {
    stations = newStations || [];
    highlightedStations = null;
    buffer = buffer || null;
    bufferValid = false;
  }

  function setup(gl) {
    const vshader = WebGL.Util.createShader(gl, gl.VERTEX_SHADER, vertexShader);
    const fshader =
        WebGL.Util.createShader(gl, gl.FRAGMENT_SHADER, fragmentShader);
    program = WebGL.Util.createProgram(gl, vshader, fshader);
    gl.deleteShader(vshader);
    gl.deleteShader(fshader);

    a_pos = gl.getAttribLocation(program, 'a_pos');
    a_pickColor = gl.getAttribLocation(program, 'a_pickColor');
    a_flags = gl.getAttribLocation(program, 'a_flags');
    u_perspective = gl.getUniformLocation(program, 'u_perspective');
    u_zoom = gl.getUniformLocation(program, 'u_zoom');
    u_offscreen = gl.getUniformLocation(program, 'u_offscreen');
    u_texture = gl.getUniformLocation(program, 'u_texture');
    u_highlightStations = gl.getUniformLocation(program, 'u_highlightStations');

    buffer = gl.createBuffer();
    bufferValid = false;

    texture = WebGL.Util.createTextureFromImage(gl, 'img/railviz/station.png');
  }

  function render(gl, perspective, zoom, isOffscreen) {
    if (stations.length == 0) {
      return;
    }

    gl.useProgram(program);
    if (!bufferValid) {
      fillBuffer(gl);
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.enableVertexAttribArray(a_pos);
    gl.vertexAttribPointer(a_pos, 2, gl.FLOAT, false, 12, 0);
    gl.enableVertexAttribArray(a_pickColor);
    gl.vertexAttribPointer(a_pickColor, 3, gl.UNSIGNED_BYTE, true, 12, 8);
    gl.enableVertexAttribArray(a_flags);
    gl.vertexAttribPointer(a_flags, 1, gl.UNSIGNED_BYTE, false, 12, 11);

    gl.uniformMatrix4fv(u_perspective, false, perspective);
    gl.uniform1f(u_zoom, zoom);
    gl.uniform1i(u_offscreen, isOffscreen);
    gl.uniform1i(u_highlightStations, highlightedStations != null);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.uniform1i(u_texture, 0);

    gl.drawArrays(gl.POINTS, 0, stations.length);

    gl.disableVertexAttribArray(a_flags);
    gl.disableVertexAttribArray(a_pickColor);
    gl.disableVertexAttribArray(a_pos);
  }

  function fillBuffer(gl) {
    var data = new Float32Array(stations.length * 3);
    var byteView = new Uint8Array(data.buffer);
    const useHighlights = highlightedStations != null;
    for (var i = 0; i < stations.length; i++) {
      const station = stations[i];
      const posBase = i * 3;
      const colorBase = 8 + i * 12;
      const pickColor = RailViz.Picking.pickIdToColor(PICKING_BASE + i);
      data[posBase] = station.pos.x;
      data[posBase + 1] = station.pos.y;
      byteView[colorBase] = pickColor[0];
      byteView[colorBase + 1] = pickColor[1];
      byteView[colorBase + 2] = pickColor[2];
      if (useHighlights && highlightedStations.indexOf(station.id) != -1) {
        byteView[colorBase + 3] = 128;
      }
    }
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.bufferData(gl.ARRAY_BUFFER, data, gl.STATIC_DRAW);
    bufferValid = true;
  }

  function getPickedStationIndex(pickId) {
    if (pickId != null) {
      const index = pickId - PICKING_BASE;
      if (index >= 0 && index < stations.length) {
        return index;
      }
    }
    return null;
  }

  function highlightStation(stationId) {
    highlightedStations = highlightedStations || [];
    highlightedStations.push(stationId);
    bufferValid = false;
  }

  return {
    init: init, setup: setup, render: render,
        getPickedStationIndex: getPickedStationIndex,
        highlightStation: highlightStation
  }
})();
