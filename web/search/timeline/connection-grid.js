import c from 'search/timeline/constants.js';

SVG.ConnectionGrid = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    hoverBegin: function(y, el, x) {
      x -= (c.INFO_HOVER_WIDTH / 2);
      x = Math.min(this.width - c.INFO_HOVER_WIDTH, Math.max(10, x));
      this.infoHover.updateContent(el.info);
      this.infoHover.moveInner(x, y);
      this.infoHover.show();
    },

    hoverEnd: function() { this.infoHover.hide(); },

    draw: function(width, height, timelineSettings, connections, conSelect) {
      this.width = width;

      this.drawTimeline(height, timelineSettings);
      this.drawConnections(width, height, timelineSettings, connections, conSelect);

      this.infoHover = new SVG.InfoHover;
      this.add(this.put(this.infoHover).draw())
    },

    drawConnections: function(width, height, timelineSettings, connections, conSelect) {
      if (connections.length === 0) {
        return;
      }

      const xPos = timelineSettings.timeToXIntercept;
      var yPos = Math.max(30, (4 - connections.length) * 50);
      connections.forEach((sections, conId) => {
        const elements = sections.map(section => new Object({
          x: xPos(section.begin),
          len: xPos(section.end) - xPos(section.begin),
          color: section.color,
          label: section.transport.move.category_name,
          info: section
        }));

        var hoverBeginFn = this.hoverBegin.bind(this, yPos + c.INFO_HOVER_OFFSET);
        var con = this.put(new SVG.Connection)
                      .draw(elements, hoverBeginFn, this.hoverEnd.bind(this))
                      .move(0, yPos)
                      .attr({'cursor': 'pointer'})
        con.click(conSelect.bind(null, conId));
        this.add(con);

        yPos += c.RADIUS * 5.5;
      });
    },

    drawTimeline: function(height, timelineSettings) {
      var t = new Date(timelineSettings.start.getTime()), x;
      for (var cut = 0; cut < timelineSettings.totalCuts; cut++) {
        x = timelineSettings.timeToXIntercept(t);
        this.add(this.put(new SVG.Line)
                     .plot(x, 0, x, height - 10)
                     .stroke({ width: 0.2, color: '#999' }));
        t.setTime(t.getTime() + timelineSettings.scale);
      }
    }
  },

  construct: {
    connectionGrid: function(width, height, timelineSettings, connections, conSelect) {
      const grid = new SVG.ConnectionGrid;
      grid.draw(width, height, timelineSettings, connections, conSelect);
      return this.put(grid);
    }
  }
});
