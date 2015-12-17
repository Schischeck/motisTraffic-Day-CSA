import SVG from 'svg.js';

import './MotisMove';

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

      let totalOffset = 0;
      let lastEnd = 0;
      let lastCorrection = 0;
      elements.forEach(el => {
        const move = new SVG.MotisMove;

        if (lastEnd !== 0) {
          // Try to reduce offset by shrinking waiting time.
          totalOffset = Math.max(0, totalOffset - (el.x - lastEnd));
        }

        // Try to reduce offset by shrinking travel time.
        const before = el.len;
        el.len -= totalOffset;
        if (el.len < 0) {
          el.len = 0;
        }

        const moveGroup = this.put(move)
                            .draw(thickness, radius, el.len, el)
                            .move(el.x + totalOffset, 0)
                            .fill(el.color);

        this.add(moveGroup);
        this.moves.push({
          svgElement: moveGroup,
          info: el.info,
          x: Math.min(250, Math.max(10, el.x + totalOffset + el.len / 2.0 - 140))
        });

        lastCorrection = before - el.len;
        totalOffset = Math.max(totalOffset + move.getOffset(), 0);
        lastEnd = el.x + el.len;
      });

      const lastEl = elements[elements.length - 1];
      const x = lastEl.x + lastEl.len + totalOffset + lastCorrection;
      const circleGroup = this.put(new SVG.G)
                            .move(x, thickness / 2)
                            .fill('#666');
      circleGroup.put(new SVG.Circle).radius(radius);
      return this;
    }
  },
  construct: {
    motiscon: function(thickness, radius, elements) {
      return this.put(new SVG.MotisConnection)
                 .draw(thickness, radius, elements);
    }
  }
});
