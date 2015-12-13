import SVG from 'svg.js';

SVG.MotisMove = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    getOffset: function() {
      return this.offset;
    },

    setDrawParameters: function(thickness, radius, desired_len) {
      this.thickness = thickness;
      this.radius = radius;

      this.in_eq_width = Math.cos(Math.asin((thickness / 2.0) / radius)) * radius;
      this.x_offset = this.in_eq_width + radius;
      this.y_offset = -thickness / 2.0 + radius;
      this.x = this.x_offset;
      this.y = this.y_offset;

      this.setDesiredLength(desired_len);
    },

    setDesiredLength: function(desired_len) {
      var minWidth = this.in_eq_width / 2.0;
      var len = desired_len - (this.radius + this.in_eq_width) - (this.thickness / 6.0);
      var offset = minWidth - len;
      if (len < minWidth) {
        len = minWidth;
      }

      this.len = len;
      this.offset = offset;
    },

    generatePathData: function() {
      return 'm' + this.in_eq_width + ',' + 0 +
             'a' + this.radius + ',' + this.radius + ' 0 1,0 0,' + this.thickness +
             'h ' + this.len +
             'v -' + this.thickness +
             'z' +
             'M' + (this.x + this.len - this.radius) + ',' + (this.y - this.y_offset) +
             'a' + this.radius + ',' + this.radius + ' 0 0,0 0,' + this.thickness;
    },

    updatePath: function() {
      this.path.plot(this.generatePathData());
    },

    updateLength: function(desired_len) {
      this.setDesiredLength(desired_len);
      this.updatePath();
    },

    draw: function(thickness, radius, desired_len, el) {
      this.setDrawParameters(thickness, radius, desired_len);

      var g = this.put(new SVG.G);
      this.path = this.put(new SVG.Path)
                      .attr({'fill-rule': 'evenodd'});
      this.updatePath();

      g.add(this.path);
      g.add(this.put(new SVG.Text())
                .text(el.label || '???')
                .attr({'cursor': 'default'})
                .attr({'fill': '#FFF'})
                .attr({'font-family': 'Verdana,sans-serif'})
                .attr({'font-weight': 'bold'})
                .attr({'text-anchor': 'middle'})
                .size(9.5)
                .move(0, -0.25 * thickness));

      return g;
    }
  },

  construct: {
    motismove: function(thickness, radius, len) {
      return this
        .put(new SVG.MotisMove)
        .draw(thickness, radius, len);
    }
  }
});
