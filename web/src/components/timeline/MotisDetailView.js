import React from 'react';

import SVG from 'svg.js';
import './ForeignObject';
import './MotisMove';
import colors from './MoveColors';

function pad(num, size) {
  const s = '000' + num;
  return s.substr(s.length - size);
}

function formatTime(time) {
  const date = new Date(time * 1000);
  return pad(date.getHours(), 2) + ':' + pad(date.getMinutes(), 2);
}

SVG.MotisDetailView = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(thickness, radius, len, totalHeight, elements) {
      const addLabel = function(text, x, y, style) {
        const stopNameDOM = document.createElement('span');
        stopNameDOM.innerHTML = text;
        const stopNameObj = this.put(new SVG.ForeignObject).size('100%', '100%');
        stopNameObj.appendChild(stopNameDOM, {style});
        stopNameObj.move(x, y);
        this.add(stopNameObj);
      }.bind(this);

      const rotateGroup = this.put(new SVG.G).move(180, radius).rotate(90);
      elements.forEach((el, i) => {
        const move = new SVG.MotisMove;
        const moveGroup = rotateGroup.put(move)
                            .draw(thickness, radius, len, el.label, -90)
                            .move(len * i, 0)
                            .fill(el.color);
        rotateGroup.add(moveGroup);

        addLabel(el.from.stop.name, 280, len * i + 0.33 * radius, 'font-size: 115%; font-weight: lighter');
        addLabel(formatTime(el.from.stop.departure.time), 115, len * i + 0.15 * radius, 'font-weight: lighter');
      });

      const finalStopEl = elements[elements.length - 1];
      addLabel(finalStopEl.to.stop.name, 280, len * elements.length, 'font-size: 115%; font-weight: lighter');

      const finalStopTimeDOM = document.createElement('span');
      addLabel(formatTime(finalStopEl.from.stop.departure.time), 115, len * elements.length, 'font-weight: lighter');

      this.add(rotateGroup);
      const x = elements.length * len;
      const circleGroup = rotateGroup.put(new SVG.G)
                            .move(x, thickness / 2)
                            .fill('#666');
      circleGroup.put(new SVG.Circle).radius(radius);

      return this;
    }
  },
  construct: {
    motisdetail: function(thickness, radius, len, totalHeight, elements) {
      return this.put(new SVG.MotisDetailView)
                 .draw(thickness, radius, len, totalHeight, elements);
    }
  }
});

export default class DeltailView extends React.Component {
  constructor(props) {
    super(props);
    this.movelen = 100;
  }

  componentWillMount() {
    this.componentWillReceiveProps(this.props);
  }

  componentDidMount() {
    if (this.grid) {
      delete this.grid;
    }
    if (this.svg) {
      delete this.svg;
    }

    this.svg = SVG('detailview').clear();
    this.grid = this.svg.motisdetail(9, 13, this.movelen, this.height, this.segments);
  }

  componentWillReceiveProps(newProps) {
    this.segments = this.connectionToDisplayParts(newProps.connection);
    const height = Math.max(this.segments.length * (this.movelen + 13), 200);
    this.setState({height});
  }

  componentDidUpdate = this.componentDidMount

  connectionToDisplayParts(c) {
    function transports(con, from, to) {
      return con.transports.filter(t => {
        return t.move.range.from >= from && t.move.range.to <= to;
      });
    }

    const walkTargets = c.transports.filter(move => {
      return move.move_type === 'Walk';
    }).map(walk => {
      return walk.move.range.to;
    });

    const importantStops = c.stops.map((stop, i) => {
      return {
        type: 'stop',
        stop,
        i
      };
    }).filter((el, i) => {
      return i === 1 || i === c.stops.length - 2 || el.stop.interchange || walkTargets.indexOf(i) !== -1;
    });

    return importantStops.reduce((acc, from, i) => {
      if (i === importantStops.length - 1) return acc;

      const to = importantStops[i + 1];
      const transport = transports(c, from.i, to.i)[0];
      if (transport && transport.move.name) {
        acc.push({
          transport,
          from,
          to,
          label: transport.move.category_name,
          color: colors[transport.move.clasz] || colors.fallback,
          begin: new Date(from.stop.departure.time * 1000),
          end: new Date(to.stop.arrival.time * 1000)
        });
      }
      return acc;
    }, []);
  }


  render() {
    return (
      <div style={{height: Math.min(300, (this.state.height)) + 'px', overflow: 'auto'}} {...this.props}>
        <div>
          <svg id="detailview" style={{height: this.state.height + 'px'}}></svg>
        </div>
      </div>
    );
  }
}
