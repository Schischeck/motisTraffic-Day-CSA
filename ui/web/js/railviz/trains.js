var RailViz = RailViz || {};

RailViz.Trains = (function() {
  const vertexShader = `
        attribute vec4 a_startPos;
        attribute vec4 a_endPos;
        attribute float a_progress;
        attribute vec4 a_color;
        attribute vec4 a_pickColor;
        
        uniform mat4 u_perspective;
        uniform float u_zoom;
        
        varying vec4 v_color;
        varying vec4 v_pickColor;

        void main() {
            if (a_progress < 0.0) {
                gl_Position = vec4(-2.0, -2.0, 0.0, 1.0);
            } else {
                gl_Position = u_perspective * mix(a_startPos, a_endPos, a_progress);
            }
            gl_PointSize = u_zoom;
            v_color = a_color;
            v_pickColor = a_pickColor;
        }
    `;

  const fragmentShader = `
        precision mediump float;
        
        uniform bool u_offscreen;
        uniform sampler2D u_texture;
        
        varying vec4 v_color;
        varying vec4 v_pickColor;
        
        void main() {
            if (u_offscreen) {
                gl_FragColor = v_pickColor;
            } else {
                gl_FragColor = v_color * texture2D(u_texture, gl_PointCoord);
            }
        }
    `;

  var trains = [];
  var routes = [];
  var positionBuffer = null;
  var progressBuffer = null;
  var colorBuffer = null;
  var elementArrayBuffer = null;
  var positionData = null;
  var positionBufferInitialized = false;
  var positionBufferUpdated = false;
  var progressBufferInitialized = false;
  var colorBufferInitialized = false;
  var texture = null;
  var filteredIndices = null;
  var isFiltered = false;
  var filterBufferInitialized = false;
  var totalFrames = 0;
  var updatedBufferFrames = 0;
  var program;
  var a_startPos;
  var a_endPos;
  var a_progress;
  var a_color;
  var a_pickColor;
  var u_perspective;
  var u_zoom;
  var u_offscreen;
  var u_texture;

  const PICKING_BASE = 0;

  function init(newTrains, newRoutes) {
    trains = newTrains || [];
    routes = newRoutes || [];
    positionData = null;
    positionBufferInitialized = false;
    progressBufferInitialized = false;
    colorBufferInitialized = false;
    isFiltered = false;
    filterBufferInitialized = false;
    filteredIndices = null;
  }

  function setup(gl) {
    const vshader = WebGL.Util.createShader(gl, gl.VERTEX_SHADER, vertexShader);
    const fshader =
        WebGL.Util.createShader(gl, gl.FRAGMENT_SHADER, fragmentShader);
    program = WebGL.Util.createProgram(gl, vshader, fshader);
    gl.deleteShader(vshader);
    gl.deleteShader(fshader);

    a_startPos = gl.getAttribLocation(program, 'a_startPos');
    a_endPos = gl.getAttribLocation(program, 'a_endPos');
    a_progress = gl.getAttribLocation(program, 'a_progress');
    a_color = gl.getAttribLocation(program, 'a_color');
    a_pickColor = gl.getAttribLocation(program, 'a_pickColor');
    u_perspective = gl.getUniformLocation(program, 'u_perspective');
    u_zoom = gl.getUniformLocation(program, 'u_zoom');
    u_offscreen = gl.getUniformLocation(program, 'u_offscreen');
    u_texture = gl.getUniformLocation(program, 'u_texture');

    positionBuffer = gl.createBuffer();
    progressBuffer = gl.createBuffer();
    colorBuffer = gl.createBuffer();
    texture = WebGL.Util.createTextureFromImage(gl, 'img/railviz/train.png');

    positionBufferInitialized = false;
    progressBufferInitialized = false;
    colorBufferInitialized = false;
    filterBufferInitialized = false;
  }

  function preRender(gl, timestamp) {
    const time = (timestamp / 1000) % 100;

    if (!positionData) {
      fillPositionBuffer();
    }

    positionBufferUpdated = false;
    trains.forEach(
        (train, trainIndex) =>
            updateCurrentSubSegment(train, trainIndex, time));

    gl.useProgram(program);
    totalFrames++;
    if (!positionBufferInitialized) {
      initAndUploadPositionBuffer(gl);
      updatedBufferFrames++;
    } else if (positionBufferUpdated) {
      uploadUpdatedPositionBuffer(gl);
      updatedBufferFrames++;
    }
    fillProgressBuffer(gl);
    if (!colorBufferInitialized) {
      fillColorBuffer(gl);
    }

    // if (totalFrames % 300 == 0) {
    //     console.log('position buffer uploaded:',
    //         updatedBufferFrames, '/', totalFrames, '=',
    //         (updatedBufferFrames / totalFrames * 100), '% of all frames');
    // }

    if (isFiltered && !filterBufferInitialized) {
      setupElementArrayBuffer(gl);
    }
  }

  function render(gl, perspective, zoom, isOffscreen) {
    var trainCount = isFiltered ? filteredIndices.length : trains.length;
    if (trainCount == 0) {
      return;
    }

    gl.useProgram(program);

    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
    gl.enableVertexAttribArray(a_startPos);
    gl.vertexAttribPointer(a_startPos, 2, gl.FLOAT, false, 16, 0);
    gl.enableVertexAttribArray(a_endPos);
    gl.vertexAttribPointer(a_endPos, 2, gl.FLOAT, false, 16, 8);

    gl.bindBuffer(gl.ARRAY_BUFFER, progressBuffer);
    gl.enableVertexAttribArray(a_progress);
    gl.vertexAttribPointer(a_progress, 1, gl.FLOAT, false, 0, 0);

    gl.bindBuffer(gl.ARRAY_BUFFER, colorBuffer);
    gl.enableVertexAttribArray(a_color);
    gl.vertexAttribPointer(a_color, 3, gl.UNSIGNED_BYTE, true, 8, 0);
    gl.enableVertexAttribArray(a_pickColor);
    gl.vertexAttribPointer(a_pickColor, 3, gl.UNSIGNED_BYTE, true, 8, 4);

    gl.uniformMatrix4fv(u_perspective, false, perspective);
    gl.uniform1f(u_zoom, zoom);
    gl.uniform1i(u_offscreen, isOffscreen);

    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.uniform1i(u_texture, 0);

    if (isFiltered) {
      gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
      gl.drawElements(gl.POINTS, trainCount, gl.UNSIGNED_SHORT, 0);
    } else {
      gl.drawArrays(gl.POINTS, 0, trainCount);
    }
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, null);

    gl.disableVertexAttribArray(a_pickColor);
    gl.disableVertexAttribArray(a_color);
    gl.disableVertexAttribArray(a_endPos);
    gl.disableVertexAttribArray(a_startPos);
  }

  function initAndUploadPositionBuffer(gl) {
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, positionData, gl.DYNAMIC_DRAW);
    positionBufferInitialized = true;
  }

  function uploadUpdatedPositionBuffer(gl) {
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
    gl.bufferSubData(gl.ARRAY_BUFFER, 0, positionData);
  }

  function fillPositionBuffer() {
    positionData = new Float32Array(trains.length * 4);
    for (var i = 0; i < trains.length; i++) {
      updatePositionBuffer(i);
    }
    positionBufferUpdated = true;
  }

  function updatePositionBuffer(trainIndex) {
    const train = trains[trainIndex];
    const subSegmentIndex = train.currentSubSegmentIndex;
    const base = trainIndex * 4;
    if (subSegmentIndex != null) {
      const segment = routes[train.route_index].segments[train.segment_index];
      const polyline = segment.coordinates.coordinates;
      const polyOffset = subSegmentIndex * 2;

      // a_startPos
      positionData[base] = polyline[polyOffset];
      positionData[base + 1] = polyline[polyOffset + 1];
      // a_endPos
      positionData[base + 2] = polyline[polyOffset + 2];
      positionData[base + 3] = polyline[polyOffset + 3];
    } else {
      // a_startPos
      positionData[base] = -100;
      positionData[base + 1] = -100;
      // a_endPos
      positionData[base + 2] = -100;
      positionData[base + 3] = -100;
    }
    positionBufferUpdated = true;
  }

  function fillProgressBuffer(gl) {
    var data = new Float32Array(trains.length);
    for (var i = 0; i < trains.length; i++) {
      const train = trains[i];
      if (train.currentSubSegmentProgress != null) {
        data[i] = train.currentSubSegmentProgress;
      } else {
        data[i] = -1.0;
      }
    }
    gl.bindBuffer(gl.ARRAY_BUFFER, progressBuffer);
    if (progressBufferInitialized) {
      gl.bufferSubData(gl.ARRAY_BUFFER, 0, data);
    } else {
      gl.bufferData(gl.ARRAY_BUFFER, data, gl.DYNAMIC_DRAW);
      progressBufferInitialized = true;
    }
  }

  function updateCurrentSubSegment(train, trainIndex, time) {
    let updated = false;
    if (time < train.d_time || time > train.a_time) {
      updated = (train.currentSubSegmentIndex != null);
      train.currentSubSegmentIndex = null;
      train.currentSubSegmentProgress = null;
    } else {
      const progress = (time - train.d_time) / (train.a_time - train.d_time);
      const segment = routes[train.route_index].segments[train.segment_index];
      const totalPosition = progress * segment.totalLength;
      if (train.currentSubSegmentIndex != null) {
        const subOffset =
            segment.subSegmentOffsets[train.currentSubSegmentIndex];
        const subLen = segment.subSegmentLengths[train.currentSubSegmentIndex];
        if (totalPosition >= subOffset &&
            totalPosition <= (subOffset + subLen)) {
          train.currentSubSegmentProgress =
              (totalPosition - subOffset) / subLen;
          return;
        }
      }
      for (let i = train.currentSubSegmentIndex || 0;
           i < segment.subSegmentOffsets.length; i++) {
        const subOffset = segment.subSegmentOffsets[i];
        const subLen = segment.subSegmentLengths[i];
        if (totalPosition >= subOffset &&
            totalPosition <= (subOffset + subLen)) {
          updated = (train.currentSubSegmentIndex !== i);
          train.currentSubSegmentIndex = i;
          train.currentSubSegmentProgress =
              (totalPosition - subOffset) / subLen;
          break;
        }
      }
    }
    if (updated) {
      updatePositionBuffer(trainIndex);
    }
  }

  function fillColorBuffer(gl) {
    var data = new Uint8Array(trains.length * 8);
    for (var i = 0; i < trains.length; i++) {
      const train = trains[i];
      const base = i * 8;
      const pickColor = RailViz.Picking.pickIdToColor(PICKING_BASE + i);

      // a_color
      data[base] = Math.floor(Math.random() * 255);
      data[base + 1] = Math.floor(Math.random() * 255);
      data[base + 2] = Math.floor(Math.random() * 255);

      // a_pickColor
      data[base + 4] = pickColor[0];
      data[base + 5] = pickColor[1];
      data[base + 6] = pickColor[2];
    }
    gl.bindBuffer(gl.ARRAY_BUFFER, colorBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, data, gl.STATIC_DRAW);
    colorBufferInitialized = true;
  }

  function setFilteredIndices(indices) {
    filteredIndices = indices;
    isFiltered = indices != null;
    filterBufferInitialized = false;
  }

  function setupElementArrayBuffer(gl) {
    if (!elementArrayBuffer || !gl.isBuffer(elementArrayBuffer)) {
      elementArrayBuffer = gl.createBuffer();
    }
    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
    gl.bufferData(
        gl.ELEMENT_ARRAY_BUFFER, new Uint16Array(filteredIndices),
        gl.STATIC_DRAW);
    filterBufferInitialized = true;
  }

  function getPickedTrainIndex(pickId) {
    if (pickId != null) {
      const index = pickId - PICKING_BASE;
      if (index >= 0 && index < trains.length) {
        return index;
      }
    }
    return null;
  }

  return {
    init: init, setup: setup, render: render, preRender: preRender,
        setFilteredIndices: setFilteredIndices,
        getPickedTrainIndex: getPickedTrainIndex
  }
})();
