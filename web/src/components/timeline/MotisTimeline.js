import React, { Component } from 'react';

import SVG from 'svg.js';

import MotisGrid from './MotisGrid';

import style from './MotisTimeline.scss';

export default class Timeline extends React.Component {
  constructor(props) {
    super(props);
  }

  getHeight() {
    return this.props.connections.length * 75;
  }

  componentDidMount() {
    if (this.grid) {
      delete this.grid;
    }
    if (this.svg) {
      delete this.svg;
    }
    this.svg = SVG('timeline').clear();
    this.grid = this.svg.motisgrid(550, this.getHeight(), []);

    function transports(con, from, to) {
      return con.transports.filter(t => {
        return t.move.range.from >= from && t.move.range.to <= to;
      }).map(t => {
        return {
          'name': t.move.category_name,
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

    this.grid.drawConnections(this.props.connections.map(c => {
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
        if (transport && transport.name) {
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
      <div className={ style.timeline }  {...this.props}>
        <svg id="timeline" style={{height: this.getHeight()}}></svg>
      </div>
    );
  }
}