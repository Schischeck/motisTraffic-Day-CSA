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

  function init(newRoutes, vertexCount) {
    routes = newRoutes || [];
    mesh = createMesh(routes, vertexCount || 0);
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
    if (isOffscreen || mesh.elementArray.length == 0) {
      return;
    }

    gl.useProgram(program);
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
        gl.TRIANGLES, mesh.elementArray.length, gl.UNSIGNED_SHORT, 0);

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

  function createMesh(routes, vertexCount) {
    if (vertexCount == 0) {
      return {
        vertexArray: new Float32Array(0),
        elementArray: new Uint16Array(0)
      };
    }

    // vertexCount = 2 * #pointsOnTheLine
    // triangleCount = (#pointsOnTheLine - 1) * 2 = vertexCount - 2
    const triangleCount = vertexCount - 2;
    const elementCount = triangleCount * 3;  // 3 vertices per triangle

    const VERTEX_SIZE = 4;
    const vertexArray = new Float32Array(vertexCount * VERTEX_SIZE);
    const elementArray = new Uint16Array(elementCount);

    let vertexIndex = 0;
    let elementIndex = 0;

    console.log(
        'createMesh:', 'vertexCount =', vertexCount, 'triangleCount =',
        triangleCount, 'elementCount =', elementCount);

    // Each segment is an independent line
    for (let routeIdx = 0; routeIdx < routes.length; routeIdx++) {
      const segments = routes[routeIdx].segments;
      for (let segIdx = 0; segIdx < segments.length; segIdx++) {
        const coords = segments[segIdx].coordinates.coordinates;
        const pointCount = coords.length / 2;
        const subsegmentCount = pointCount - 1;
        const firstVertexIndex = vertexIndex;

        // calculate normalized per segment direction vectors (used to calculate
        // per segment normals)
        const directionVectors = new Array(subsegmentCount * 2);
        for (let i = 0; i < subsegmentCount; i++) {
          const base = i * 2;
          const x0 = coords[base], y0 = coords[base + 1], x1 = coords[base + 2],
                y1 = coords[base + 3];
          // direction vector
          const dx = x1 - x0;
          const dy = y1 - y0;
          // normalized direction vector
          const dlen = Math.sqrt(dx * dx + dy * dy);
          directionVectors[base] = dx / dlen;
          directionVectors[base + 1] = dy / dlen;
        }

        // points -> vertices (2 vertices per point)
        for (let i = 0; i < pointCount; i++) {
          const prevSubsegment = i - 1;
          const nextSubsegment = i;

          let dx, dy;
          if (prevSubsegment == -1) {
            // first point of the polyline
            dx = directionVectors[nextSubsegment * 2];
            dy = directionVectors[nextSubsegment * 2 + 1];

          } else if (nextSubsegment == subsegmentCount) {
            // last point of the polyline
            dx = directionVectors[prevSubsegment * 2];
            dy = directionVectors[prevSubsegment * 2 + 1];
          } else {
            const dx0 = directionVectors[prevSubsegment * 2],
                  dy0 = directionVectors[prevSubsegment * 2 + 1],
                  dx1 = directionVectors[nextSubsegment * 2],
                  dy1 = directionVectors[nextSubsegment * 2 + 1];
            // average normalized vector
            const nx = dx0 + dx1, ny = dy0 + dy1;
            const nlen = Math.sqrt(nx * nx + ny * ny);
            dx = nx / nlen;
            dy = ny / nlen;
          }

          const x = coords[i * 2], y = coords[i * 2 + 1];

          vertexArray[vertexIndex] = x;
          vertexArray[vertexIndex + 1] = y;
          vertexArray[vertexIndex + 2] = -dy;
          vertexArray[vertexIndex + 3] = dx;
          vertexIndex += 4;

          vertexArray[vertexIndex] = x;
          vertexArray[vertexIndex + 1] = y;
          vertexArray[vertexIndex + 2] = dy;
          vertexArray[vertexIndex + 3] = -dx;
          vertexIndex += 4;
        }

        for (let i = 0; i < subsegmentCount; i++) {
          const base = firstVertexIndex / 4 + i * 2;
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

    return {vertexArray: vertexArray, elementArray: elementArray};
  }

  return { init: init, setup: setup, render: render }
})();
