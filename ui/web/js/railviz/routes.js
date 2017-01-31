var RailViz = RailViz || {};

RailViz.Routes = (function() {
  const vertexShader = `
        attribute vec2 a_pos;
        attribute vec2 a_normal;
        
        uniform mat4 u_perspective;
        uniform float u_width;
        
        void main() {
            vec2 delta = a_normal * u_width;
            gl_Position = u_perspective * vec4(a_pos + delta, 0.0, 1.0);
        }
    `;

  const fragmentShader = `
        precision mediump float;
        
        void main() {
            // gl_FragColor = vec4(0.4, 0.4, 0.4, 0.8);
            gl_FragColor = vec4(0.4, 0.4, 0.4, 1.0);
        }
    `;

  var routes = [];
  var vertexCount = 0;
  var mesh;
  var buffer = null;
  var elementArrayBuffer = null;
  var bufferValid = false;
  var lineCount = 0;
  var program;
  var a_pos;
  var a_normal;
  var u_perspective;
  var u_width;

  function init(newRoutes, newVertexCount) {
    routes = newRoutes || [];
    mesh = null;
    vertexCount = newVertexCount || 0;
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
    u_perspective = gl.getUniformLocation(program, 'u_perspective');
    u_width = gl.getUniformLocation(program, 'u_width');

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
      mesh = createMesh(gl, routes, vertexCount);
    }

    if (!bufferValid) {
      fillBuffer(gl);
    }

    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.enableVertexAttribArray(a_pos);
    gl.vertexAttribPointer(a_pos, 2, gl.FLOAT, false, 16, 0);
    gl.enableVertexAttribArray(a_normal);
    gl.vertexAttribPointer(a_normal, 2, gl.FLOAT, false, 16, 8);

    gl.uniformMatrix4fv(u_perspective, false, perspective);
    // gl.uniform1f(u_width, (performance.now() % 500) / 500000);
    // gl.uniform1f(u_width, 0.00001 * zoom);
    // gl.uniform1f(u_width, 0.0001);
    // gl.uniform1f(u_width, 0.0014 / zoom);
    // gl.uniform1f(u_width, lineWidth);
    const lineWidth = zoom == 18 ? 0.00002 : 0.00001 * Math.pow(2, 18 - zoom);
    // gl.uniform1f(u_width, lineWidth);
    gl.uniform1f(u_width, lineWidth * (performance.now() % 1000) / 500);

    gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, elementArrayBuffer);
    gl.drawElements(
        gl.TRIANGLES, mesh.elementArray.length, mesh.elementArrayType, 0);

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

  function createMesh(gl, routes, vertexCount) {
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

    const VERTEX_SIZE = 4;
    const vertexArray = new Float32Array(vertexCount * VERTEX_SIZE);
    const uintExt = gl.getExtension('OES_element_index_uint');
    const elementArrayType =
        (elementCount > 65535 && uintExt) ? gl.UNSIGNED_INT : gl.UNSIGNED_SHORT;
    const elementArray = elementArrayType == gl.UNSIGNED_INT ?
        new Uint32Array(elementCount) :
        new Uint16Array(elementCount);

    let vertexIndex = 0;
    let elementIndex = 0;

    console.log(
        'createMesh:', 'vertexCount =', vertexCount, 'triangleCount =',
        triangleCount, 'elementCount =', elementCount, 'elementArrayType =',
        (elementArrayType == gl.UNSIGNED_INT ? 'UNSIGNED_INT' :
                                               'UNSIGNED_SHORT'));

    // Each segment is an independent line
    for (let routeIdx = 0; routeIdx < routes.length; routeIdx++) {
      const segments = routes[routeIdx].segments;
      for (let segIdx = 0; segIdx < segments.length; segIdx++) {
        const coords = segments[segIdx].coordinates.coordinates;
        const pointCount = coords.length / 2;
        const subsegmentCount = pointCount - 1;
        const firstVertexIndex = vertexIndex;

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
              miterLen = Math.min(2.0, 1 / (nx * nnx + ny * nny));
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
          vertexIndex += VERTEX_SIZE;

          vertexArray[vertexIndex] = x;
          vertexArray[vertexIndex + 1] = y;
          vertexArray[vertexIndex + 2] = nx * -miterLen;
          vertexArray[vertexIndex + 3] = ny * -miterLen;
          vertexIndex += VERTEX_SIZE;
        }

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
      }
    }

    return {
      vertexArray: vertexArray,
      elementArray: elementArray,
      elementArrayType: elementArrayType
    };
  }

  return { init: init, setup: setup, render: render }
})();
