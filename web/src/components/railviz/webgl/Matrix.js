export class Matrix {
  constructor() {
    this.data = new Float32Array(16);
    this.zoom = 27;
  }

  fromArray(array) {
    this.data.set(array);
  }

  set(m) {
    this.data.set(m.data);
  }

  translate(tx, ty) {
    this.data[12] += this.data[0] * tx + this.data[4] * ty;
    this.data[13] += this.data[1] * tx + this.data[5] * ty;
    this.data[14] += this.data[2] * tx + this.data[6] * ty;
    this.data[15] += this.data[3] * tx + this.data[7] * ty;
  }

  scale(scaleX, scaleY) {
    this.data[0] *= scaleX;
    this.data[1] *= scaleX;
    this.data[2] *= scaleX;
    this.data[3] *= scaleX;

    this.data[4] *= scaleY;
    this.data[5] *= scaleY;
    this.data[6] *= scaleY;
    this.data[7] *= scaleY;
  }

  multiply(vec) {
    let vector = new Array();
    for (let i = 0; i < vec.length; i++)
      vector.push(vec[i]);
    if (vector.length < 4)
      vector.push(1);
    let result = new Array();
    for (let i = 0; i < 4; i++) {
      result[i] = (this.data[i] * vector[0]) + (this.data[i + 4] * vector[1]) + (this.data[i + 8] * vector[2]) + (this.data[i + 12] * vector[3]);
    }
    return result;
  }
}
