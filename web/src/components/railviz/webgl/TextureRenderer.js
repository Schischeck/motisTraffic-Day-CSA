import { Shader } from './Shader';

export class TextureRenderer {

  constructor(gl) {
    this.program = this.compileShaderProgram(gl, Shader.texture.vertex, Shader.texture.fragment);
  }

  render(gl, buffer, texture, matrix, zoom) {
    gl.useProgram(this.program);
    let attrLoc = gl.getAttribLocation(this.program, 'worldCoord');
    let matrixLoc = gl.getUniformLocation(this.program, 'mapMatrix');
    let scaleLoc = gl.getUniformLocation(this.program, 'zoom');
    gl.uniform1f(scaleLoc, parseFloat(zoom));
    gl.uniformMatrix4fv(matrixLoc, false, new Float32Array(matrix.data));
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.enableVertexAttribArray(attrLoc);
    gl.vertexAttribPointer(attrLoc, 2, gl.FLOAT, false, 0, 0);
    gl.activeTexture(gl.TEXTURE0);
    gl.bindTexture(gl.TEXTURE_2D, texture);
    gl.uniform1i(gl.getUniformLocation(this.program, 'texture'), 0);
    gl.drawArrays(gl.POINTS, 0, buffer.itemNum);
  }

  compileShaderProgram(gl, vertexShaderSRC, fragmentShaderSRC) {
    let program = null;
    let vertexShader = gl.createShader(gl.VERTEX_SHADER);
    gl.shaderSource(vertexShader, vertexShaderSRC);
    gl.compileShader(vertexShader);
    let fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    gl.shaderSource(fragmentShader, fragmentShaderSRC);
    gl.compileShader(fragmentShader);
    // link shaders to create our program
    program = gl.createProgram();
    gl.attachShader(program, vertexShader);
    gl.attachShader(program, fragmentShader);
    gl.linkProgram(program);
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
      console.error(gl.getProgramInfoLog(program));
    }
    return program;
  }
}
