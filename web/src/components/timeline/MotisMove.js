import SVG from 'svg.js';

SVG.MotisMove = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    getOffset: function() {
      return this.offset;
    },

    draw: function(thickness, radius, desired_len, label) {
      var in_eq_width = Math.cos(Math.asin((thickness / 2.0) / radius)) * radius;
      var x_offset = in_eq_width + radius;
      var y_offset = -thickness / 2.0 + radius;
      var x = x_offset;
      var y = y_offset;
      var len = desired_len;
      len -= radius + in_eq_width;
      len -= thickness / 6.0;

      var minWidth = in_eq_width / 2.0;
      this.offset = minWidth - len;
      if (len < minWidth) {
        len = minWidth;
      }

      var g = new SVG.G;
      var p = new SVG.Path;
      p.plot('m' + in_eq_width + ',' + 0 +
             'a' + radius + ',' + radius + ' 0 1,0 0,' + thickness +
             'h ' + len +
             'v -' + thickness +
             'z' +
             'M' + (x + len - radius) + ',' + (y - y_offset) +
             'a' + radius + ',' + radius + ' 0 0,0 0,' + thickness);
      p.attr({'fill-rule': 'evenodd'});

      g.add(p);
      g.add(new SVG.Text().text(label || '???')
                          .attr({'fill': '#FFF'})
                          .attr({'font-family': 'Verdana'})
                          .attr({'font-weight': 'bold'})
                          .attr({'text-anchor': 'middle'})
                          .size(9.5)
                          .move(0, -0.45 * thickness));

      return this.put(g);
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