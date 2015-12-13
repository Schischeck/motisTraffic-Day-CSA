import SVG from 'svg.js';

import ForeignObject from './ForeignObject'
import MotisConnection from './MotisConnection';

import TimelineCalculator from './TimelineCalculator';

function pad(num, size) {
  var s = '000' + num;
  return s.substr(s.length-size);
}

function formatTime(time) {
  const date = new Date(time * 1000);
  return pad(date.getHours(), 2) + ':' + pad(date.getMinutes(), 2);
}

SVG.MotisGrid = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    updateInfoHoverContent: function(el) {
      console.log(el);

      const from = el.from.stop;
      const to = el.to.stop;
      const transport = el.transport.move;

      const departure = {
        time: formatTime(from.departure.time),
        isDelayed: from.departure.time != from.departure.schedule_time,
        track: from.departure.platform
      };

      const arrival = {
        time: formatTime(to.arrival.time),
        isDelayed: to.arrival.time != to.arrival.schedule_time,
        track: to.arrival.platform
      };

      this.infoHover.innerHTML =
            '<table style="padding: 3px 5px; margin: 0">' +
              '<tr>' +
                '<td>' + from.name + '</td>' +
                '<td>' +
                  '<span style="' + (departure.isDelayed ? 'color: #A33' : '') + '">' +
                    departure.time +
                  '</span>' +
                '</td>' +
                '<td>' + (departure.track ? 'tr. ' + departure.track : '') + '</td>' +
                '<td style="border-left: dashed 1px #CCC; padding-left: 8px; text-align: right" rowspan="2">' +
                  '<span style="font-weight: bold">' + transport.name + '</span>' +
                '</td>' +
              '</tr>' +
              '<tr>' +
                '<td>' + to.name + '</td>' +
                '<td>' +
                  '<span style="' + (arrival.isDelayed ? 'color: #A33' : '') + '">' +
                    arrival.time +
                  '</span>' +
                '</td>' +
                '<td>' + (arrival.track ? 'tr. ' + arrival.track : '') + '</td>' +
              '</tr>' +
             '</table>';
    },

    updateInfoHoverPosition: function(x, y) {
      this.infoHover.style.left = x + 'px';
      this.infoHover.style.top = y + 'px';
    },

    showInfoHover: function() {
      this.infoHover.style.visibility = 'visible';
    },

    hideInfoHover: function() {
      this.infoHover.style.visibility = 'hidden';
    },

    createInfoHover: function() {
      if (!this.infoHover) {
        var foreignObject = this.put(new SVG.ForeignObject);
        foreignObject.size(250, 20);

        this.infoHover = document.createElement('div');

        var style = '';
        style += 'box-shadow: 0 3px 14px rgba(0,0,0,0.4);';
        style += 'width: 100%;';
        style += 'position: relative;'
        style += 'font-family: sans-serif;';
        style += 'color: #999;';
        style += 'font-weight: lighter;';
        style += 'font-size: .7em;';
        style += 'border-radius: 2px;';
        style += 'background-color: white;';
        style += 'border: 1px solid #BBB';
        style += 'visibility: hidden';

        foreignObject.appendChild(this.infoHover, {'style': style});
        this.add(foreignObject);

        this.updateInfoHoverPosition(100, 100);
      }
    },

    drawConnections: function(cons, timelineSettings) {
      if (this.drawedConnections) {
        this.drawedConnections.forEach(c => { c.remove(); });
      }
      this.drawedConnections = [];

      this.createInfoHover();

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
            label: section.transport.move.category_name,
            info: section
          });
        }
        newCon.draw(this.settings.thickness, this.settings.radius, elements);
        newCon.move(0, y);
        this.add(newCon);
        this.drawedConnections.push(newCon);

        newCon.onHoverBegin(function(y, el, x) {
          this.updateInfoHoverContent(el);
          this.updateInfoHoverPosition(x, y);
          this.showInfoHover();
        }.bind(this, y + 20));
        newCon.onHoverEnd(function(el, x, y) {
          this.hideInfoHover();
        }.bind(this));

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
