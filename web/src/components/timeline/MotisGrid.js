require('svg.js');

var MINUTE = 60*1000;

function pad(num, size) {
  var s = '000' + num;
  return s.substr(s.length-size);
}

SVG.MotisMove = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(thickness, radius, len, label) {
      var in_eq_width = Math.cos(Math.asin((thickness / 2.0) / radius)) * radius;
      var x_offset = in_eq_width + radius;
      var y_offset = -thickness / 2.0 + radius;
      var x = x_offset;
      var y = y_offset;
      len -= radius + in_eq_width;
      len -= thickness / 4.0;

      var g = new SVG.G;

      var p = new SVG.Path;
      p.plot('m0,0 ' +
             'a' + radius + ',' + radius + ' 0 1,0 0,' + thickness +
             'h ' + len +
             'v -' + thickness +
             'z' +
             'M' + (x + len - x_offset) + ',' + (y - y_offset) +
             'a' + radius + ',' + radius + ' 0 0,0 0,' + thickness);
      p.attr({'fill-rule': 'evenodd'});

      g.add(p);
      g.add(new SVG.Text().text(label)
                          .attr({'fill': '#555'})
                          .attr({'font-family': 'Verdana'})
                          .size(12)
                          .move(5, 0.5 * thickness));

      return this.put(g);
    }
  },
  construct: {
    motismove: function(thickness, radius, len) {
      return this
        .put(new SVG.MotisMove)
        .draw(thickness, radius, len);
    }
  }
});

SVG.MotisConnection = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(thickness, radius, elements) {
      var self = this;
      elements.forEach(function(el) {
        self.add(self.put(new SVG.MotisMove)
                     .draw(thickness, radius, el.len, el.label)
                     .move(el.x, 0)
                     .fill(el.color));
      });
      var lastEl = elements[elements.length - 1];
      var x = lastEl.x + lastEl.len;
      var circleGroup = this.put(new SVG.G)
                            .move(x - radius, thickness / 2)
                            .fill('#555');
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

    connections: function(cons) {
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

      var y = 100;
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

        y += this.settings.radius * 5;
      }
    },

    drawTimeline: function(begin, end) {
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

      this.settings.begin = new Date(t.getTime());
      this.settings.end = new Date(t.getTime() + totalCuts * scale);

      for (var cut = 0; cut < totalCuts - 1; cut++) {
        t.setTime(t.getTime() + scale);
        x = this.timeToXIntercept(t);
        this.add(this.put(new SVG.Text)
                     .text(pad(t.getHours(), 2) + ':' + pad(t.getMinutes(), 2))
                     .size(15)
                     .move(x - 20, 10));
        this.add(this.put(new SVG.Line)
                     .plot(x, 30, x, this.settings.height - 10)
                     .stroke({ width: 0.2, color: '#999' }));
      }
    }
  },
  construct: {
    motisgrid: function(width, height, connections, padding, thickness, radius) {
      var grid = new SVG.MotisGrid;
      grid.settings = {};
      grid.settings.thickness = thickness || 8;
      grid.settings.radius = radius || 10;
      grid.settings.padding = padding || grid.settings.radius;
      grid.settings.width = width;
      grid.settings.height = height;

      var g = this.put(grid);
      g.add(this.put(new SVG.Rect)
                .size(width, height)
                .opacity(0.05));

      grid.connections(connections);

      return g;
    }
  }
});