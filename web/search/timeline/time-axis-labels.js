SVG.TimeAxisLabels = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(timelineSettings) {
      var t = new Date(timelineSettings.start.getTime());
      for (var cut = 0; cut < timelineSettings.totalCuts; cut++) {
        this.add(this.put(new SVG.Text)
                     .text(formatTime(t))
                     .attr({'font-family': 'Verdana,sans-serif'})
                     .attr({'text-anchor': 'middle'})
                     .size(9)
                     .move(timelineSettings.timeToXIntercept(t), 0));
        t.setTime(t.getTime() + timelineSettings.scale);
      }
      return this;
    }
  },

  construct: {
    timeAxisLabels: function(timelineSettings) {
      return this.put(new SVG.TimeAxisLabels).draw(timelineSettings);
    }
  }
});
