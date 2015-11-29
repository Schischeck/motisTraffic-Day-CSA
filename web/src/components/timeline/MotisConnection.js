SVG.MotisConnection = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(thickness, radius, elements) {
      var self = this;
      elements.forEach(function(el) {
        self.add(self.put(new SVG.MotisMove)
                     .draw(thickness, radius, el.len, el.label)
                     .move(el.x, 0)
                     .fill(el.color));
      });
      var lastEl = elements[elements.length - 1];
      var x = lastEl.x + lastEl.len;
      var circleGroup = this.put(new SVG.G)
                            .move(x - radius, thickness / 2)
                            .fill('#555');
      circleGroup.put(new SVG.Circle).radius(radius);
      return this;
    }
  },
  construct: {
    motiscon: function(thickness, radius, elements) {
      return this.put(new MotisConnection)
                 .draw(thickness, radius, elements);
    }
  }
});