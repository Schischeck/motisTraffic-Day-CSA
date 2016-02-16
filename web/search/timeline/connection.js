import c from 'search/timeline/constants.js';

SVG.Connection = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(elements, hoverBegin, hoverEnd) {
      var totalOffset = 0;
      var lastEnd = 0;
      var lastCorrection = 0;
      elements.forEach(el => {
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

        const newLength = el.len - c.CIRCLE_WIDTH;
        const adjustedLength = Math.max(newLength, c.MIN_LENGTH);
        const offset = Math.max(0, c.MIN_LENGTH - newLength);

        const moveGroup = this.put(new SVG.Move)
                              .draw(adjustedLength, el.label, el.color)
                              .fill(el.color)
                              .move(el.x + totalOffset, 0);
        moveGroup.mouseover(() => hoverBegin(el, el.x + totalOffset + adjustedLength / 2));
        moveGroup.mouseout(hoverEnd);
        this.add(moveGroup);

        lastCorrection = before - el.len;
        totalOffset = Math.max(totalOffset + offset, 0);
        lastEnd = el.x + el.len;
      });

      const lastEl = elements[elements.length - 1];
      const arrivalCircleX = lastEl.x + lastEl.len + totalOffset;
      this.add(this.put(new SVG.Circle)
                   .move(arrivalCircleX, c.THICKNESS / 2)
                   .fill('#666')
                   .radius(c.RADIUS));
      return this;
    }
  },
  construct: {
    connection: function(elements, hoverBegin, hoverEnd) {
      return this.put(new SVG.connection)
                 .draw(elements, hoverBegin, hoverEnd);
    }
  }
});
