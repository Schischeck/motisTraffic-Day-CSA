export class Texture {

  constructor() {
    this.id = 0;
  }


  createCircle(rad, fillColor, borderColor, borderThickness) {
    let cv = document.createElement('canvas');
    cv.width = rad * 2 + borderThickness * 2;
    cv.height = rad * 2 + borderThickness * 2;
    let ct2d = cv.getContext('2d');
    ct2d.beginPath();
    ct2d.arc(rad + borderThickness, rad + borderThickness, rad, 0, 2 * Math.PI, false);
    ct2d.fillStyle = 'rgba(' + fillColor[0] + ',' + fillColor[1] + ',' + fillColor[2] + ',' + fillColor[3] + ')';
    ct2d.fill();
    ct2d.lineWidth = borderThickness;
    ct2d.strokeStyle = 'rgba(' + borderColor[0] + ',' + borderColor[1] + ',' + borderColor[2] + ',' + borderColor[3] + ')';
    ct2d.stroke();
    cv.id = this.id++;
    return cv;
  }

}

const instance = new Texture();
export default instance;
