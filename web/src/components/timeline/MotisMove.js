SVG.MotisMove = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(thickness, radius, len, label) {
      var in_eq_width = Math.cos(Math.asin((thickness / 2.0) / radius)) * radius;
      var x_offset = in_eq_width + radius;
      var y_offset = -thickness / 2.0 + radius;
      var x = x_offset;
      var y = y_offset;
      len -= radius + in_eq_width;
      len -= thickness / 4.0;

      var g = new SVG.G;

      var p = new SVG.Path;
      p.plot('m0,0 ' +
             'a' + radius + ',' + radius + ' 0 1,0 0,' + thickness +
             'h ' + len +
             'v -' + thickness +
             'z' +
             'M' + (x + len - x_offset) + ',' + (y - y_offset) +
             'a' + radius + ',' + radius + ' 0 0,0 0,' + thickness);
      p.attr({'fill-rule': 'evenodd'});

      g.add(p);
      g.add(new SVG.Text().text(label)
                          .attr({'fill': '#555'})
                          .attr({'font-family': 'Verdana'})
                          .size(12)
                          .move(5, 0.5 * thickness));

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