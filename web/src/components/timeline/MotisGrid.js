import SVG from 'svg.js';

import MotisConnection from './MotisConnection';

import TimelineCalculator from './TimelineCalculator';

SVG.MotisGrid = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    drawConnections: function(cons, timelineSettings) {
      if (this.drawedConnections) {
        this.drawedConnections.forEach(c => { c.remove(); });
      }
      this.drawedConnections = [];

      if (cons.length == 0) {
        return;
      }

      this.drawTimeline(timelineSettings);

      var y = Math.max(20, (3 - cons.length) * 50);
      for (var i = 0; i < cons.length; i++) {
        var newCon = this.put(new SVG.MotisConnection);
        var c = cons[i];
        var elements = [];
        for (var j = 0; j < c.length; j++) {
          var section = c[j];
          elements.push({
            x: this.timeline.timeToXIntercept(section.begin),
            len: this.timeline.timeToXIntercept(section.end) - this.timeline.timeToXIntercept(section.begin),
            color: section.color,
            label: section.label
          });
        }
        newCon.draw(this.settings.thickness, this.settings.radius, elements)
        newCon.move(0, y);
        this.add(newCon);
        this.drawedConnections.push(newCon);

        y += this.settings.radius * 5.5;
      }
    },

    drawTimeline: function(timelineSettings) {
      if (this.timelineElements) {
        this.timelineElements.forEach(c => { c.remove(); });
      }
      this.timelineElements = [];

      this.settings.begin = new Date(timelineSettings.begin.getTime());
      this.settings.end = new Date(timelineSettings.end.getTime());
      this.timeline = timelineSettings;

      var t = new Date(this.timeline.start.getTime());
      var totalCuts = this.timeline.totalCuts;
      var scale = this.timeline.scale;

      for (var cut = 0; cut < totalCuts - 1; cut++) {
        t.setTime(t.getTime() + scale);
        var x = this.timeline.timeToXIntercept(t);
        var line = this.put(new SVG.Line)
                       .plot(x, 0, x, this.settings.height - 10)
                       .stroke({ width: 0.2, color: '#999' });
        this.add(line);
        this.timelineElements.push(line);
      }
    }
  },

  construct: {
    motisgrid: function(width, height, connections, padding, thickness, radius) {
      var grid = new SVG.MotisGrid;
      grid.settings = {};
      grid.settings.thickness = thickness || 9;
      grid.settings.radius = radius || 13;
      grid.settings.padding = grid.settings.radius * 4;
      grid.settings.width = width;
      grid.settings.height = height;

      var g = this.put(grid);
      grid.drawConnections(connections);

      return g;
    }
  }
});
