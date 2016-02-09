RailViz = function(map){
  this.div = map.getPanes().overlayPane;
  this.map = map;
  this.initCanvas();
}

RailViz.prototype.initCanvas = function() {
  this.canvas = document.createElement('canvas');
  this.div.appendChild(this.canvas);
  this.canvas.width = '100%';
  this.canvas.height = '100%';
  this.canvas.style.position = 'absolute';
  this.canvas.left = 0;
  this.canvas.top = 0;
  var ctx = this.canvas.getContext("2d");
  ctx.fillStyle = "#FF0000";
  ctx.fillRect(0,0,150,75);
}