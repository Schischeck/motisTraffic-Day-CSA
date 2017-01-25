var WebGL = WebGL || {};

WebGL.Util = (function() {

    function createShader(gl, type, src) {
        var shader = gl.createShader(type);
        gl.shaderSource(shader, src);
        gl.compileShader(shader);
        if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS) && !gl.isContextLost()) {
            console.log(gl.getShaderInfoLog(shader));
            gl.deleteShader(shader);
            return null;
        }
        return shader;
    }

    function createProgram(gl, vertexShader, fragementShader) {
        var program = gl.createProgram();
        gl.attachShader(program, vertexShader);
        gl.attachShader(program, fragementShader);
        gl.linkProgram(program);
        if (!gl.getProgramParameter(program, gl.LINK_STATUS) && !gl.isContextLost()) {
            console.log(gl.getProgramInfoLog(program));
            gl.deleteProgram(program);
            return null;
        }
        return program;
    }

    function resizeCanvasToDisplaySize(canvas, multiplier) {
        multiplier = multiplier || 1;
        var width  = canvas.clientWidth  * multiplier | 0;
        var height = canvas.clientHeight * multiplier | 0;
        if (canvas.width !== width ||  canvas.height !== height) {
            canvas.width  = width;
            canvas.height = height;
            return true;
        }
        return false;
    }

    function makeOrtho(left, right, bottom, top, znear, zfar) {
        return [
            2 / (right - left),
            0,
            0,
            0,

            0,
            2 / (top - bottom),
            0,
            0,

            0,
            0,
            -2 / (zfar - znear),
            0,

            -(right+left)/(right-left),
            -(top+bottom)/(top-bottom),
            -(zfar+znear)/(zfar-znear),
            1
        ];
    }

    function makeOrtho2D(left, right, bottom, top) {
        return makeOrtho(left, right, bottom, top, -1, 1);
    }

    function m4Scale(m, v) {
        const x = v[0];
        const y = v[1];
        const z = v[2];

        m[0] *= x;
        m[1] *= x;
        m[2] *= x;
        m[3] *= x;
        m[4] *= y;
        m[5] *= y;
        m[6] *= y;
        m[7] *= y;
        m[8] *= z;
        m[9] *= z;
        m[10] *= z;
        m[11] *= z;
    }

    function m4Translate(m, v) {
        const x = v[0];
        const y = v[1];
        const z = v[2];

        m[12] += m[0] * x + m[4] * y + m[8] * z;
        m[13] += m[1] * x + m[5] * y + m[9] * z;
        m[14] += m[2] * x + m[6] * y + m[10] * z;
        m[15] += m[3] * x + m[7] * y + m[11] * z;
    }

    function createTextureFromImage(gl, url) {
        var texture = gl.createTexture();
        gl.bindTexture(gl.TEXTURE_2D, texture);
        // placeholder: 1x1 white
        gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, gl.UNSIGNED_BYTE, new Uint8Array([255, 255, 255, 255]));
        var img = new Image();
        img.addEventListener('load', () => {
            gl.bindTexture(gl.TEXTURE_2D, texture);
            gl.pixelStorei(gl.UNPACK_FLIP_Y_WEBGL, true);
            gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, img);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
            gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
            // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.LINEAR);
            // gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR_MIPMAP_LINEAR);
            gl.generateMipmap(gl.TEXTURE_2D);
            gl.bindTexture(gl.TEXTURE_2D, null);
        });
        img.addEventListener('error', () => {
            console.log('Could not load texture:', url);
        });
        img.src = url;
        return texture;
    }

    return {
        createShader: createShader,
        createProgram: createProgram,
        resizeCanvasToDisplaySize: resizeCanvasToDisplaySize,
        makeOrtho: makeOrtho,
        makeOrtho2D: makeOrtho2D,
        m4Scale: m4Scale,
        m4Translate: m4Translate,
        createTextureFromImage: createTextureFromImage
    };

})();
