import c from 'search/timeline/constants.js';

function correctionY(labelRotation, isIcon) {
  if (isIcon) {
    return 0;
  }
  if (labelRotation) {
    return 1;
  } else {
    return c.MOVE_LABEL_Y_OFFSET;
  }
}

SVG.Move = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(length, label, color, labelRotation, isIcon) {
      const g = this.put(new SVG.G);
      g.add(this.put(new SVG.Path)
                .attr({'fill-rule': 'evenodd'})
                .plot('m' + c.CUT_RADIUS + ',0' +
                      'a' + c.RADIUS + ',' + c.RADIUS + ' 0 1,0 0,' + c.THICKNESS +
                      'h ' + length +
                      'v -' + c.THICKNESS +
                      'z' +
                      'M' + (c.X_OFFSET + length - c.RADIUS) + ',0' +
                      'a' + c.RADIUS + ',' + c.RADIUS + ' 0 0,0 0,' + c.THICKNESS));

      g.add(this.put(new SVG.Text())
                .text(label || '???')
                .attr({'cursor': 'pointer'})
                .attr({'fill': '#FFF'})
                .attr({'font-family': isIcon ? 'Material Icons' : 'Verdana,sans-serif'})
                .attr({'font-weight': isIcon ? 'normal' : 'bold'})
                .attr({'text-anchor': 'middle'})
                .move(0, correctionY(labelRotation, isIcon))
                .size(isIcon ? 2 * c.THICKNESS : c.THICKNESS)
                .rotate(labelRotation || 0));

      return g;
    }
  }
});
