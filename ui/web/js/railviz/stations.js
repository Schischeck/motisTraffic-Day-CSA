var RailViz = RailViz || {};

RailViz.Stations = (function() {
  const vertexShader = `
        attribute vec2 a_pos;
        attribute vec4 a_pickColor;
        
        uniform mat4 u_perspective;
        uniform float u_zoom;
        
        varying vec4 v_pickColor;
        
        void main() {
            gl_Position = u_perspective * vec4(a_pos, 0.0, 1.0);
            gl_PointSize = u_zoom - 4.0;
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
  var buffer = null;
  var bufferValid = false;
  var texture = null;
  var program;
  var a_pos;
  var a_pickColor;
  var u_perspective;
  var u_zoom;
  var u_offscreen;
  var u_texture;

  const PICKING_BASE = 100000;

  function init(newStations) {
    stations = newStations || [];
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
    u_perspective = gl.getUniformLocation(program, 'u_perspective');
    u_zoom = gl.getUniformLocation(program, 'u_zoom');
    u_offscreen = gl.getUniformLocation(program, 'u_offscreen');
    u_texture = gl.getUniformLocation(program, 'u_texture');

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

    gl.uniformMatrix4fv(u_perspective, false, perspective);
    gl.uniform1f(u_zoom, zoom);
    gl.uniform1i(u_offscreen, isOffscreen);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.uniform1i(u_texture, 0);

    gl.drawArrays(gl.POINTS, 0, stations.length);

    gl.disableVertexAttribArray(a_pos);
  }

  function fillBuffer(gl) {
    var data = new Float32Array(stations.length * 3);
    var byteView = new Uint8Array(data.buffer);
    for (var i = 0; i < stations.length; i++) {
      const posBase = i * 3;
      const colorBase = 8 + i * 12;
      const pickColor = RailViz.Picking.pickIdToColor(PICKING_BASE + i);
      data[posBase] = stations[i].pos.x;
      data[posBase + 1] = stations[i].pos.y;
      byteView[colorBase] = pickColor[0];
      byteView[colorBase + 1] = pickColor[1];
      byteView[colorBase + 2] = pickColor[2];
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

  return {
    init: init, setup: setup, render: render,
        getPickedStationIndex: getPickedStationIndex
  }
})();
