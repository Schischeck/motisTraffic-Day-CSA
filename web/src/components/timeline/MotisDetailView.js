import React, { Component } from 'react';

import SVG from 'svg.js';
import ForeignObject from './ForeignObject'

import MotisMove from './MotisMove';

SVG.MotisDetailView = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(thickness, radius, len, totalHeight, elements) {
      const rotateGroup = this.put(new SVG.G);
      elements.forEach((el ,i) => {
        let move = new SVG.MotisMove;
        let moveGroup = rotateGroup.put(move)
                            .draw(thickness, radius, len, el.label, -90)
                            .move(len * i, 0)
                            .fill(el.color);
        rotateGroup.add(moveGroup)
            .move(radius * 2 + 100, radius)
            .rotate(90, 0, 0);

        const stopNameDOM = document.createElement('span');
        stopNameDOM.innerHTML = el.name;
        let stopNameObj = this.put(new SVG.ForeignObject).size('100%', '100%');
        stopNameObj.appendChild(stopNameDOM, {style: "font-size: 115%; font-weight: lighter"});
        stopNameObj.move(300, len * i);
        this.add(stopNameObj);

        const stopTimeDOM = document.createElement('span');
        stopTimeDOM.innerHTML = '12:34';
        let stopTimeObj = this.put(new SVG.ForeignObject).size('100%', '100%');
        stopTimeObj.appendChild(stopTimeDOM, {style: "font-weight: lighter"});
        stopTimeObj.move(0, len * i);
        this.add(stopTimeObj);
      });
      this.add(rotateGroup);
      const lastEl = elements[elements.length - 1];
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
    this.height = Math.max(200, this.props.connection.length * this.movelen + 54)
  }

  componentDidMount() {
    if (this.grid) {
      delete this.grid;
    }
    if (this.svg) {
      delete this.svg;
    }
    this.svg = SVG('detailview').clear();
    this.grid = this.svg.motisdetail(9, 13, this.movelen, this.height, this.props.connection);
  }

  componentDidUpdate = this.componentDidMount

  render() {
    return (
      <div style={{height: Math.min(300, (this.height + 10)) + 'px', overflow: 'auto'}}  {...this.props}>
        <div>
          <svg id="detailview" style={{height: this.height + 'px'}}></svg>
        </div>
      </div>
    );
  }
}
