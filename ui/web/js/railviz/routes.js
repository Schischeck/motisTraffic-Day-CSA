var RailViz = RailViz || {};

RailViz.Routes = (function() {
  const vertexShader = `
        attribute vec4 a_pos;
        attribute vec2 a_normal;
        attribute vec4 a_color;
        attribute float a_flags;
        
        uniform vec2 u_resolution;
        uniform mat4 u_perspective;
        uniform float u_width;
        uniform bool u_useHighlighting;

        varying vec4 v_color;

        const vec4 defaultColor = vec4(0.4, 0.4, 0.4, 1.0);
        
        void main() {
            float flags = a_flags;

            if (flags >= 128.0) {
              v_color = a_color;
              flags -= 128.0;
            } else {
              v_color = defaultColor;
            }

            bool highlighted = false;
            if (flags >= 64.0) {
              highlighted = true;
              // flags -= 64.0;
            }

            vec4 base = u_perspective * a_pos;
            float width = u_width;
            if (u_useHighlighting) {
              if (highlighted) {
                width *= 1.5;
              } else {
                width *= 0.5;
                v_color.a *= 0.9;
              }
            }
            vec2 offset = width * a_normal / u_resolution;

            gl_Position = base + vec4(offset, 0.0, 0.0);
        }
    `;

  const fragmentShader = `
        precision mediump float;

        varying vec4 v_color;
        
        void main() {
            gl_FragColor = v_color;
        }
    `;

  var routes;
  var routeVertexCount;
  var footpaths;
  var footpathVertexCount;
  var vertexCount;
  var mesh;
  var coloredSegments;
  var highlightedSegments;
  var useHighlighting;
  var buffer = null;
  var elementArrayBuffer = null;
  var bufferValid = false;
  var lineCount = 0;
  var program;
  var a_pos;
  var a_normal;
  var a_color;
  var a_flags;
  var u_resolution;
  var u_perspective;
  var u_width;
  var u_useHighlighting;

  const VERTEX_SIZE = 5;  // in 32-bit floats

  function init(newRoutes, newRouteVertexCount, newFootpaths) {
    routes = newRoutes || [];
    routeVertexCount = newRouteVertexCount || 0;
    footpaths = newFootpaths || [];
    footpathVertexCount =
        footpaths.reduce((a, s) => a + s.coordinates.coordinates.length, 0);
    vertexCount = routeVertexCount + footpathVertexCount;
    mesh = null;
    coloredSegments = null;
    highlightedSegments = null;
    useHighlighting = false;
    buffer = buffer || null;
    bufferValid = false;
    lineCount = 0;
  }

  function setup(gl) {
    const vshader = WebGL.Util.createShader(gl, gl.VERTEX_SHADER, vertexShader);
    const fshader =
        WebGL.Util.createShader(gl, gl.FRAGMENT_SHADER, fragmentShader);
    program = WebGL.Util.createProgram(gl, vshader, fshader);
    gl.deleteShader(vshader);
    gl.deleteShader(fshader);

    a_pos = gl.getAttribLocation(program, 'a_pos');
    a_normal = gl.getAttribLocation(program, 'a_normal');
    a_color = gl.getAttribLocation(program, 'a_color');
    a_flags = gl.getAttribLocation(program, 'a_flags');
    u_resolution = gl.getUniformLocation(program, 'u_resolution');
    u_perspective = gl.getUniformLocation(program, 'u_perspective');
    u_width = gl.getUniformLocation(program, 'u_width');
    u_useHighlighting = gl.getUniformLocation(program, 'u_useHighlighting');

    buffer = gl.createBuffer();
    elementArrayBuffer = gl.createBuffer();
    bufferValid = false;
  }

  function render(gl, perspective, zoom, isOffscreen) {
    if (isOffscreen || vertexCount == 0) {
      return;
    }

    gl.useProgram(program);

    if (!mesh) {
      mesh = createMesh(gl);
      if (coloredSegments) {
        coloredSegments.forEach(args => colorSegment.apply(this, args));
      }
      if (highlightedSegments) {
        highlightedSegments.forEach(args => highlightSegment.apply(this, args));
      }
    }

    if (!bufferValid) {
      fillBuffer(gl);
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.enableVertexAttribArray(a_pos);
    gl.vertexAttribPointer(a_pos, 2, gl.FLOAT, false, 20, 0);
    gl.enableVertexAttribArray(a_normal);
    gl.vertexAttribPointer(a_normal, 2, gl.FLOAT, false, 20, 8);
    gl.enableVertexAttribArray(a_color);
    gl.vertexAttribPointer(a_color, 3, gl.UNSIGNED_BYTE, true, 20, 16);
    gl.enableVertexAttribArray(a_flags);
    gl.vertexAttribPointer(a_flags, 1, gl.UNSIGNED_BYTE, false, 20, 19);

    // -height because y axis is flipped in webgl (-1 is at the bottom)
    gl.uniform2f(u_resolution, gl.canvas.width, -gl.canvas.height);
    gl.uniformMatrix4fv(u_perspective, false, perspective);
    gl.uniform1f(u_width, zoom > 8 ? 4.0 : 2.0);
    gl.uniform1i(u_useHighlighting, useHighlighting);

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
    gl.drawElements(
        gl.TRIANGLES, mesh.elementArray.length, mesh.elementArrayType, 0);

    gl.disableVertexAttribArray(a_flags);
    gl.disableVertexAttribArray(a_color);
    gl.disableVertexAttribArray(a_normal);
    gl.disableVertexAttribArray(a_pos);
  }

  function fillBuffer(gl) {
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.bufferData(gl.ARRAY_BUFFER, mesh.vertexArray, gl.STATIC_DRAW);

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
    gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, mesh.elementArray, gl.STATIC_DRAW);

    bufferValid = true;
  }

  function createMesh(gl) {
    if (vertexCount == 0) {
      return {
        vertexArray: new Float32Array(0),
        elementArray: new Uint16Array(0),
        elementArrayType: gl.UNSIGNED_SHORT
      };
    }

    // vertexCount = 2 * #pointsOnTheLine
    // triangleCount = (#pointsOnTheLine - 1) * 2 = vertexCount - 2
    const triangleCount = vertexCount - 2;
    const elementCount = triangleCount * 3;  // 3 vertices per triangle

    const vertexArray = new Float32Array(vertexCount * VERTEX_SIZE);
    const byteView = new Uint8Array(vertexArray.buffer);
    const uintExt = gl.getExtension('OES_element_index_uint');
    const elementArrayType =
        (elementCount > 65535 && uintExt) ? gl.UNSIGNED_INT : gl.UNSIGNED_SHORT;
    const elementArray = elementArrayType == gl.UNSIGNED_INT ?
        new Uint32Array(elementCount) :
        new Uint16Array(elementCount);

    let vertexIndex = 0;
    let elementIndex = 0;

    routes.forEach(route => route.segments.forEach(segment => {
      const r = createSegmentMesh(
          segment, vertexArray, vertexIndex, elementArray, elementIndex, false,
          byteView);
      vertexIndex = r.nextVertexIndex;
      elementIndex = r.nextElementIndex;
    }));

    footpaths.forEach(footpath => {
      const r = createSegmentMesh(
          footpath, vertexArray, vertexIndex, elementArray, elementIndex, true,
          byteView);
      vertexIndex = r.nextVertexIndex;
      elementIndex = r.nextElementIndex;
    });

    return {
      vertexArray: vertexArray,
      elementArray: elementArray,
      elementArrayType: elementArrayType
    };
  }

  function createSegmentMesh(
      segment, vertexArray, firstVertexIndex, elementArray, firstElementIndex,
      isFootpath, byteView) {
    const coords = segment.coordinates.coordinates;
    const pointCount = coords.length / 2;
    const subsegmentCount = pointCount - 1;
    let vertexIndex = firstVertexIndex;
    let elementIndex = firstElementIndex;

    // calculate unit normals for each segment
    const normals = new Array(subsegmentCount * 2);
    for (let i = 0; i < subsegmentCount; i++) {
      const base = i * 2;
      const x0 = coords[base], y0 = coords[base + 1], x1 = coords[base + 2],
            y1 = coords[base + 3];
      // direction vector
      const dx = x1 - x0;
      const dy = y1 - y0;
      // normalized direction vector
      const dlen = Math.sqrt(dx * dx + dy * dy);
      normals[base] = dlen != 0 ? -dy / dlen : 0;
      normals[base + 1] = dlen != 0 ? dx / dlen : 0;
    }

    const setFlags =
        function() {
      if (isFootpath) {
        const colorBase = vertexIndex * 4 + 16;
        byteView[colorBase] = 0x33;
        byteView[colorBase + 1] = 0x33;
        byteView[colorBase + 2] = 0x33;
        byteView[colorBase + 3] = (128 | 64);
      }
    }

    // points -> vertices (2 vertices per point)
    for (let i = 0; i < pointCount; i++) {
      const prevSubsegment = i - 1;
      const nextSubsegment = i;

      let nx, ny;
      let miterLen = 1.0;

      if (prevSubsegment == -1) {
        // first point of the polyline
        nx = normals[nextSubsegment * 2];
        ny = normals[nextSubsegment * 2 + 1];

      } else if (nextSubsegment == subsegmentCount) {
        // last point of the polyline
        nx = normals[prevSubsegment * 2];
        ny = normals[prevSubsegment * 2 + 1];
      } else {
        const pnx = normals[prevSubsegment * 2],
              pny = normals[prevSubsegment * 2 + 1],
              nnx = normals[nextSubsegment * 2],
              nny = normals[nextSubsegment * 2 + 1];
        // average normals
        const anx = pnx + nnx, any = pny + nny;
        const nlen = Math.sqrt(anx * anx + any * any);
        if (nlen > 0) {
          nx = nlen != 0 ? anx / nlen : 0;
          ny = nlen != 0 ? any / nlen : 0;
          miterLen = Math.max(0.5, Math.min(2.0, 1 / (nx * nnx + ny * nny)));
        } else {
          nx = pnx;
          ny = pny;
          miterLen = 1.0;
        }
      }

      const x = coords[i * 2], y = coords[i * 2 + 1];

      vertexArray[vertexIndex] = x;
      vertexArray[vertexIndex + 1] = y;
      vertexArray[vertexIndex + 2] = nx * miterLen;
      vertexArray[vertexIndex + 3] = ny * miterLen;
      setFlags();
      vertexIndex += VERTEX_SIZE;

      vertexArray[vertexIndex] = x;
      vertexArray[vertexIndex + 1] = y;
      vertexArray[vertexIndex + 2] = nx * -miterLen;
      vertexArray[vertexIndex + 3] = ny * -miterLen;
      setFlags();
      vertexIndex += VERTEX_SIZE;
    }

    segment.firstVertexIndex = firstVertexIndex;
    segment.lastVertexIndex = vertexIndex - VERTEX_SIZE;

    for (let i = 0; i < subsegmentCount; i++) {
      const base = firstVertexIndex / VERTEX_SIZE + i * 2;
      // 1st triangle
      elementArray[elementIndex++] = base;
      elementArray[elementIndex++] = base + 1;
      elementArray[elementIndex++] = base + 3;
      // 2nd triangle
      elementArray[elementIndex++] = base;
      elementArray[elementIndex++] = base + 3;
      elementArray[elementIndex++] = base + 2;
    }

    return {nextVertexIndex: vertexIndex, nextElementIndex: elementIndex};
  }

  function colorSegment(routeIdx, segmentIdx, color) {
    if (!mesh) {
      coloredSegments = coloredSegments || [];
      coloredSegments.push([].slice.call(arguments));
      return;
    }
    const segment = routes[routeIdx].segments[segmentIdx];
    const byteView = new Uint8Array(mesh.vertexArray.buffer);
    for (let vertexIndex = segment.firstVertexIndex;
         vertexIndex <= segment.lastVertexIndex; vertexIndex += VERTEX_SIZE) {
      const colorBase = vertexIndex * 4 + 16;
      byteView[colorBase] = color[0];
      byteView[colorBase + 1] = color[1];
      byteView[colorBase + 2] = color[2];
      byteView[colorBase + 3] = (byteView[colorBase + 3] | 128);
    }
    bufferValid = false;
  }

  function highlightSegment(routeIdx, segmentIdx) {
    if (!mesh) {
      highlightedSegments = highlightedSegments || [];
      highlightedSegments.push([].slice.call(arguments));
      return;
    }
    const segment = routes[routeIdx].segments[segmentIdx];
    const byteView = new Uint8Array(mesh.vertexArray.buffer);
    for (let vertexIndex = segment.firstVertexIndex;
         vertexIndex <= segment.lastVertexIndex; vertexIndex += VERTEX_SIZE) {
      const flagOffset = vertexIndex * 4 + 16 + 3;
      byteView[flagOffset] = (byteView[flagOffset] | 64);
    }
    bufferValid = false;
    useHighlighting = true;
  }

  return {
    init: init, setup: setup, render: render, colorSegment: colorSegment,
        highlightSegment: highlightSegment
  }
})();
