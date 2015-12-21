import React from 'react';

import SVG from 'svg.js';
import './MotisGrid';
import './SVGTimeLabels';
import timelineCalculator from './TimelineCalculator';

import colors from './MoveColors';

import style from './MotisTimeline.scss';

export default class Timeline extends React.Component {
  constructor(props) {
    super(props);
  }

  componentDidMount() {
    if (this.grid) {
      delete this.grid;
    }
    if (this.timelineLabelsSVG) {
      delete this.timelineLabelsSVG;
    }
    if (this.timelineSVG) {
      delete this.timelineSVG;
    }
    this.timelineLabelsSVG = SVG('timeline-labels').clear();
    this.timelineSVG = SVG('timeline').clear();
    this.grid = this.timelineSVG.motisgrid(520, this.getHeight(), []);
    this.timelineLabels = this.timelineLabelsSVG.motistimelabels(520, 20, []);

    function transports(con, from, to) {
      return con.transports.filter(t => {
        return t.move.range.from >= from && t.move.range.to <= to;
      });
    }

    const cons = this.props.connections.map(c => {
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

      const elements = [];
      for (let i = 0; i < importantStops.length - 1; i++) {
        const from = importantStops[i];
        const to = importantStops[i + 1];
        const transport = transports(c, from.i, to.i)[0];
        if (transport && transport.move.name) {
          elements.push({
            transport,
            from,
            to,
            color: colors[transport.move.clasz] || colors.fallback,
            begin: new Date(from.stop.departure.time * 1000),
            end: new Date(to.stop.arrival.time * 1000)
          });
        }
      }
      return elements;
    });

    const settings = timelineCalculator(cons, 520, 60);
    this.grid.drawConnections(cons, settings, this.props.onConnectionSelected);
    this.timelineLabels.drawTimeline(settings);
  }

  componentDidUpdate = this.componentDidMount

  getHeight() {
    return Math.max(200, this.props.connections.length * 72.5) + 50;
  }

  render() {
    return (
      <div style={{marginTop: '10px'}}>
        <svg style={{height: '20px'}} id="timeline-labels"></svg>
        <div className={ style.timeline } style={{height: Math.min(300, (this.getHeight() + 10)) + 'px'}}>
          <svg id="timeline" style={{height: this.getHeight() + 'px'}}></svg>
        </div>
      </div>
    );
  }
}
