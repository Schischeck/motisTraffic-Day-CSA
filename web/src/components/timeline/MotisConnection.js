import SVG from 'svg.js';

import MotisMove from './MotisMove';

SVG.MotisConnection = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(thickness, radius, elements) {
      var self = this;
      var totalOffset = 0;
      var lastEnd = 0;
      elements.forEach(function(el) {
        let move = new SVG.MotisMove;

        if (lastEnd != 0) {
          // Try to reduce offset by shrinking waiting time.
          let before = totalOffset;
          totalOffset = Math.max(0, totalOffset - (el.x - lastEnd));
          let diff = totalOffset - before;
        }

        // Try to reduce offset by shrinking travel time.
        lastEnd = el.x + el.len;
        let before = el.len;
        el.len -= totalOffset;
        if (el.len < 0) {
          el.len = 0;
        }

        let moveGroup = self.put(move)
                            .draw(thickness, radius, el.len, el.label)
                            .move(el.x + totalOffset, 0)
                            .fill(el.color);
        totalOffset = Math.max(totalOffset + move.getOffset(), 0);
        self.add(moveGroup);
      });

      var lastEl = elements[elements.length - 1];
      var x = lastEl.x + lastEl.len + totalOffset;
      var circleGroup = this.put(new SVG.G)
                            .move(x, thickness / 2)
                            .fill('#666');
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