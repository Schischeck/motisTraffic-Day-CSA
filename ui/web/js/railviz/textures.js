var RailViz = RailViz || {};

RailViz.Textures = (function() {

  function createCircle(fillColor, borderColor, baseBorderThickness) {
    return function(size) {
      const borderThickness = baseBorderThickness / 64 * size;
      const rad = size / 2 - borderThickness;
      var cv = document.createElement('canvas');
      cv.width = size;
      cv.height = size;
      var ctx = cv.getContext('2d', {alpha: true});
      ctx.beginPath();
      ctx.arc(
          rad + borderThickness, rad + borderThickness, rad, 0, 2 * Math.PI,
          false);
      ctx.fillStyle = 'rgba(' + fillColor[0] + ',' + fillColor[1] + ',' +
          fillColor[2] + ',' + fillColor[3] + ')';
      ctx.fill();
      ctx.lineWidth = borderThickness;
      ctx.strokeStyle = 'rgba(' + borderColor[0] + ',' + borderColor[1] + ',' +
          borderColor[2] + ',' + borderColor[3] + ')';
      ctx.stroke();
      return cv;
    };
  }

  function createTrain() {
    return function(size) {
      const border = (2 / 64) * size;
      const padding = (size - (size / 2)) / 2 + border;
      const innerSize = size - (2 * padding);
      const mid = size / 2;
      var cv = document.createElement('canvas');
      cv.width = size;
      cv.height = size;
      var ctx = cv.getContext('2d', {alpha: true});

      ctx.beginPath();

      ctx.moveTo(padding, mid - (innerSize / 4));
      ctx.lineTo(mid, mid - (innerSize / 4));
      ctx.lineTo(size - padding, mid);
      ctx.lineTo(mid, mid + (innerSize / 4));
      ctx.lineTo(padding, mid + (innerSize / 4));
      ctx.closePath();

      ctx.fillStyle = 'rgba(255, 255, 255, 1.0)';
      ctx.fill();
      ctx.lineWidth = border;
      ctx.strokeStyle = 'rgba(160, 160, 160, 1.0)';
      ctx.stroke();
      return cv;
    };
  }

  return {createCircle: createCircle, createTrain: createTrain};
})();
