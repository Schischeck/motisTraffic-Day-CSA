import SVG from 'svg.js/dist/svg.js';

import React, { Component } from 'react';

import MotisConnection from './MotisConnection';
import MotisMove from './MotisMove';

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

        y += this.settings.radius * 7;
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

      this.settings.begin = new Date(t.getTime());
      this.settings.end = new Date(t.getTime() + totalCuts * scale);

      for (var cut = 0; cut < totalCuts - 1; cut++) {
        t.setTime(t.getTime() + scale);
        var x = this.timeToXIntercept(t);
        var label = this.put(new SVG.Text)
                        .text(pad(t.getHours(), 2) + ':' + pad(t.getMinutes(), 2))
                        .size(15)
                        .move(x - 20, 10);
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
      grid.settings.thickness = thickness || 8;
      grid.settings.radius = radius || 10;
      grid.settings.padding = 0;
      grid.settings.width = width;
      grid.settings.height = height;

      var g = this.put(grid);
      g.add(this.put(new SVG.Rect)
                .radius(10)
                .size(width, height)
                .opacity(0.05));

      grid.drawConnections(connections);

      return g;
    }
  }
});

export default class Timeline extends React.Component {
  constructor(props) {
    super(props);
  }

  componentDidMount() {
    const svg = SVG('timeline').clear();
    const grid = svg.motisgrid(800, 500, []);

    function transports(con, from, to) {
      return con.transports.filter(t => {
        return t.move.range.from >= from && t.move.range.to <= to;
      }).map(t => {
        return {
          'name': t.move.name,
          'clasz': t.move.clasz
        };
      });
    }

    const colors = {
      1: '#FF0000',
      2: '#708D91',
      3: '#19DD89',
      4: '#FD8F3A',
      5: '#94A507',
      6: '#F62A07',
      7: '#563AC9',
      8: '#4E070D',
      9: '#7ED3FD',
    };

    grid.drawConnections(this.props.connections.map(c => {
      const walkTargets = c.transports.filter(move => {
        return move.move_type == 'Walk';
      }).map(walk => {
        return walk.move.range.to;
      });

      let importantStops = c.stops.map((stop, i) => {
        return {
          type: 'stop',
          stop,
          i
        };
      }).filter((el, i) => {
        return i === 1 || i === c.stops.length - 2 || el.stop.interchange || walkTargets.indexOf(i) != -1;
      });

      let elements = [];
      for (let i = 0; i < importantStops.length - 1; i++) {
        let from = importantStops[i];
        let to = importantStops[i + 1];
        let transport = transports(c, from.i, to.i)[0];
        if (transport.name) {
          elements.push({
            label: transport.name,
            color: colors[transport.clasz] || '#D31996',
            begin: new Date(from.stop.departure.time * 1000),
            end: new Date(to.stop.arrival.time * 1000)
          });
        }
      }
      return elements;
    }));
  }

  componentDidUpdate = this.componentDidMount

  render() {
    return (
    <svg id="timeline" style={{'marginTop': '20px', 'width': '800px', 'height': '500px'}}></svg>
    );
  }
}
