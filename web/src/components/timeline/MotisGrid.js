import SVG from 'svg.js';

import MotisConnection from './MotisConnection';

var MINUTE = 60*1000;

function pad(num, size) {
  var s = '000' + num;
  return s.substr(s.length-size);
}

SVG.MotisGrid = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    timeToXIntercept: function(t) {
      var period = this.settings.end.getTime() - this.settings.begin.getTime();
      var until_t = t.getTime() - this.settings.begin.getTime();
      var percent = until_t / period;
      return this.settings.padding + (this.settings.width - this.settings.padding * 2) * percent;
    },

    drawConnections: function(cons) {
      if (this.drawedConnections) {
        this.drawedConnections.forEach(c => { c.remove(); });
      }
      this.drawedConnections = [];

      if (cons.length == 0) {
        return;
      }

      var begin = cons[0][0].begin;
      var end = cons[0][cons[0].length - 1].end;
      for (var i = 0; i < cons.length; i++) {
        var c = cons[i];
        for (var j = 0; j < c.length; j++) {
          if (c[j].begin < begin) {
            begin = c[j].begin;
          }
          if (c[j].end > end) {
            end = c[j].end;
          }
        }
      }

      this.drawTimeline(begin, end);

      var y = 50;
      for (var i = 0; i < cons.length; i++) {
        var newCon = new SVG.MotisConnection;
        var c = cons[i];
        var elements = [];
        for (var j = 0; j < c.length; j++) {
          var section = c[j];
          elements.push({
            x: this.timeToXIntercept(section.begin),
            len: this.timeToXIntercept(section.end) - this.timeToXIntercept(section.begin),
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

    drawTimeline: function(begin, end) {
      if (this.timelineElements) {
        this.timelineElements.forEach(c => { c.remove(); });
      }
      this.timelineElements = [];

      function getScale(begin, end) {
        var scales = [
          { start: 0, end: 5, scale: 1*MINUTE },
          { start: 5, end: 10, scale: 2*MINUTE },
          { start: 10, end: 30, scale: 5*MINUTE },
          { start: 30, end: 90, scale: 10*MINUTE },
          { start: 90, end: 180, scale: 30*MINUTE },
          { start: 3*60, end: 6*60, scale: 60*MINUTE },
          { start: 6*60, end: 12*60, scale: 120*MINUTE },
          { start: 12*60, end: 48*60, scale: 240*MINUTE },
        ];

        var minutes = (end.getTime() - begin.getTime()) / (60*1000);
        var scale = 480*MINUTE;
        for (var i = 0; i < scales.length; i++) {
          var s = scales[i];
          if (minutes >= s.start && minutes < s.end) {
            scale = s.scale;
            break;
          }
        }

        return scale;
      }

      function roundToScale(t, scale) {
        t.setSeconds(0, 0);

        var midnight = new Date(begin.getTime());
        midnight.setHours(0, 0, 0, 0);

        var sinceMidnight = t.getTime() - midnight.getTime();
        t.setTime(t.getTime() - (sinceMidnight % scale));

        return t;
      }

      var scale = getScale(begin, end);
      var t = roundToScale(new Date(begin.getTime()), scale);
      var totalCuts = Math.ceil((end.getTime() - t.getTime()) / scale);

      this.settings.begin = new Date(begin.getTime());
      this.settings.end = new Date(end.getTime());

      for (var cut = 0; cut < totalCuts - 1; cut++) {
        t.setTime(t.getTime() + scale);
        var x = this.timeToXIntercept(t);
        var label = this.put(new SVG.Text)
                        .text(pad(t.getHours(), 2) + ':' + pad(t.getMinutes(), 2))
                        .attr({'font-family': 'Verdana,sans-serif'})
                        .attr({'text-anchor': 'middle'})
                        .size(9)
                        .move(x, 10);
        var line = this.put(new SVG.Line)
                       .plot(x, 30, x, this.settings.height - 10)
                       .stroke({ width: 0.2, color: '#999' });

        this.add(label);
        this.add(line);

        this.timelineElements.push(label);
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
