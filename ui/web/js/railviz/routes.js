var RailViz = RailViz || {};

RailViz.Routes = (function() {
    const vertexShader = `
        attribute vec2 a_pos;
        
        uniform mat4 u_perspective;
        
        void main() {
            gl_Position = u_perspective * vec4(a_pos, 0.0, 1.0);
        }
    `;

    const fragmentShader = `
        precision mediump float;
        
        void main() {
            gl_FragColor = vec4(0.4, 0.4, 0.4, 0.8);
        }
    `;
    
    var routes = [];
    var buffer = null;
    var bufferValid = false;
    var lineCount = 0;
    var program;
    var a_pos;
    var u_perspective;

    function init(newRoutes) {
        routes = newRoutes || [];
        buffer = buffer || null;
        bufferValid = false;
        lineCount = 0;
    }

    function setup(gl) {
        const vshader = WebGL.Util.createShader(gl, gl.VERTEX_SHADER, vertexShader);
        const fshader = WebGL.Util.createShader(gl, gl.FRAGMENT_SHADER, fragmentShader);
        program = WebGL.Util.createProgram(gl, vshader, fshader);
        gl.deleteShader(vshader);
        gl.deleteShader(fshader);

        a_pos = gl.getAttribLocation(program, "a_pos");
        u_perspective = gl.getUniformLocation(program, "u_perspective");

        buffer = gl.createBuffer();
        bufferValid = false;
    }

    function render(gl, perspective, isOffscreen) {
        if (isOffscreen) {
            return;
        }

        gl.useProgram(program);
        if (!bufferValid) {
            fillBuffer(gl);
        }

        if (lineCount == 0) {
            return;
        }

        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.enableVertexAttribArray(a_pos);
        gl.vertexAttribPointer(a_pos, 2, gl.FLOAT, false, 0, 0);

        gl.uniformMatrix4fv(u_perspective, false, perspective);

        gl.drawArrays(gl.LINES, 0, lineCount);

        gl.disableVertexAttribArray(a_pos);
    }

    function fillBuffer(gl) {
        var points = [];
        for (var routeIdx = 0; routeIdx < routes.length; routeIdx++) {
            const segments = routes[routeIdx].segments;
            for (var segIdx = 0; segIdx < segments.length; segIdx++) {
                const coords = segments[segIdx].coordinates.coordinates;
                for (var i = 0; i < coords.length; i += 2) {
                    points.push(coords[i], coords[i + 1]);
                    if (i > 0 && i < coords.length - 2) {
                        points.push(coords[i], coords[i + 1]);
                    }
                }
            }
        }

        var data = new Float32Array(points);
        gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
        gl.bufferData(gl.ARRAY_BUFFER, data, gl.STATIC_DRAW);
        bufferValid = true;
        lineCount = points.length / 2;
    }

    return {
        init: init,
        setup: setup,
        render: render
    }
})();

