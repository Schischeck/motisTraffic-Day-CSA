import SVG from 'svg.js';

import MotisMove from './MotisMove';

SVG.MotisConnection = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    onHoverBegin: function(fun) {
      this.moves.forEach(function(move) {
        move.svgElement.mouseover(function() {
          fun(move.info, move.x);
        });
      });
    },

    onHoverEnd: function(fun) {
      this.moves.forEach(function(move) {
        move.svgElement.mouseout(fun);
      });
    },

    draw: function(thickness, radius, elements) {
      this.moves = [];

      var self = this;
      var totalOffset = 0;
      var lastEnd = 0;
      var lastCorrection = 0;
      elements.forEach(function(el) {
        let move = new SVG.MotisMove;

        if (lastEnd != 0) {
          // Try to reduce offset by shrinking waiting time.
          let before = totalOffset;
          totalOffset = Math.max(0, totalOffset - (el.x - lastEnd));
          let diff = totalOffset - before;
        }

        // Try to reduce offset by shrinking travel time.
        let before = el.len;
        el.len -= totalOffset;
        if (el.len < 0) {
          el.len = 0;
        }

        let moveGroup = self.put(move)
                            .draw(thickness, radius, el.len, el)
                            .move(el.x + totalOffset, 0)
                            .fill(el.color);

        self.add(moveGroup);
        this.moves.push({
          svgElement: moveGroup,
          info: el.info,
          x: el.x + totalOffset - 50
        });

        lastCorrection  = before - el.len;
        totalOffset = Math.max(totalOffset + move.getOffset(), 0);
        lastEnd = el.x + el.len;
      }.bind(this));

      var lastEl = elements[elements.length - 1];
      var x = lastEl.x + lastEl.len + totalOffset + lastCorrection;
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