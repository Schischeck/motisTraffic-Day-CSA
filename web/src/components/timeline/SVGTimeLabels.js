import SVG from 'svg.js';

import MotisConnection from './MotisConnection';

import TimelineCalculator from './TimelineCalculator';

var MINUTE = 60*1000;

function pad(num, size) {
  var s = '000' + num;
  return s.substr(s.length-size);
}

SVG.TimeLabels = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    drawTimeline: function(timelineSettings) {
      if (this.timelineElements) {
        this.timelineElements.forEach(c => { c.remove(); });
      }
      this.timelineElements = [];

      var t = new Date(timelineSettings.start.getTime());
      var totalCuts = timelineSettings.totalCuts;
      var scale = timelineSettings.scale;

      for (var cut = 0; cut < totalCuts - 1; cut++) {
        t.setTime(t.getTime() + scale);
        var x = timelineSettings.timeToXIntercept(t);
        var label = this.put(new SVG.Text)
                        .text(pad(t.getHours(), 2) + ':' + pad(t.getMinutes(), 2))
                        .attr({'font-family': 'Verdana,sans-serif'})
                        .attr({'text-anchor': 'middle'})
                        .size(9)
                        .move(x, 10);
        this.add(label);
        this.timelineElements.push(label);
      }
    }
  },

  construct: {
    motistimelabels: function() {
      return this.put(new SVG.TimeLabels);
    }
  }
});
