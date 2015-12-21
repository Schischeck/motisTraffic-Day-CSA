import { Shader } from './Shader';

export class ShapeRenderer {

  constructor(gl) {
    this.program = this.compileShaderProgram(gl, Shader.lines.vertex, Shader.lines.fragment);
  }

  render(gl, buffer, texture, metaData, matrix) {
    gl.useProgram(this.program);
    let attrLoc = gl.getAttribLocation(this.program, 'worldCoord');
    let matrixLoc = gl.getUniformLocation(this.program, 'mapMatrix');
    gl.uniformMatrix4fv(matrixLoc, false, new Float32Array(matrix.data));
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.enableVertexAttribArray(attrLoc);
    gl.vertexAttribPointer(attrLoc, 2, gl.FLOAT, false, 0, 0);
    if (texture === gl.LINES) {
      for (let i = 0; i < metaData.length; i++) {
        gl.drawArrays(gl.LINES, metaData[i].start, metaData[i].size);
      }
    }
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
