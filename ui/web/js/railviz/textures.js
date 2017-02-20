var RailViz = RailViz || {};

RailViz.Textures = (function() {

  function createCircle(fillColor, borderColor, baseBorderThickness) {
    return function(size) {
      const borderThickness = baseBorderThickness / 64 * size;
      const rad = size / 2 - borderThickness;
      var cv = document.createElement('canvas');
      cv.width = size;
      cv.height = size;
      var ct2d = cv.getContext('2d', {alpha: true});
      ct2d.beginPath();
      ct2d.arc(
          rad + borderThickness, rad + borderThickness, rad, 0, 2 * Math.PI,
          false);
      ct2d.fillStyle = 'rgba(' + fillColor[0] + ',' + fillColor[1] + ',' +
          fillColor[2] + ',' + fillColor[3] + ')';
      ct2d.fill();
      ct2d.lineWidth = borderThickness;
      ct2d.strokeStyle = 'rgba(' + borderColor[0] + ',' + borderColor[1] + ',' +
          borderColor[2] + ',' + borderColor[3] + ')';
      ct2d.stroke();
      cv.id = this.id++;
      return cv;
    };
  }

  return {createCircle: createCircle};
})();
