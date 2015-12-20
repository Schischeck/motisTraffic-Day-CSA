import React from 'react';

import SVG from 'svg.js';
import './ForeignObject';
import './MotisMove';
import colors from './MoveColors';

import iconcss from '../MaterialIcons.scss';
const materialicons = iconcss['material-icons'];

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
      const addLabel = function(text, x, y, style, width, height) {
        width = width || '100%';
        height = height || '100%';
        style['width'] = width;
        style['height'] = height;
        style['display'] = 'table-cell';
        style['vertical-align'] = 'middle';

        let styleStr = '';
        for (let key in style) {
          styleStr += key + ':' + style[key] + ';';
        }

        const stopNameDOM = document.createElement('div');
        stopNameDOM.innerHTML = text;
        const stopNameObj = this.put(new SVG.ForeignObject).size(width, height);
        stopNameObj.appendChild(stopNameDOM, {style: styleStr});
        stopNameObj.move(x, y);
        this.add(stopNameObj);
      }.bind(this);

      const addTimeLabel = function(pos, arrivalTime, departureTime) {
        let text = '';
        if (arrivalTime) {
          text += ' ' + formatTime(arrivalTime);
        }
        if (departureTime) {
          text += ' ' + formatTime(departureTime);
        }
        addLabel(text, 115, len * pos, {
          'font-size': '75%',
          'color': '#555'
        }, '40px', '2.5em');
      }.bind(this);

      const addStationLabel = function(text, pos) {
        addLabel(text, 260, len * pos - 2.5, {
          'font-size': '90%',
          'font-weight': 'lighter'
        }, '220px', '2.5em');
      }.bind(this);

      const addTrackLabel = function(pos, showArrival, arrivalTrack, showDeparture, departureTrack) {
        let text = '';
        if (showArrival) {
          if (arrivalTrack) {
            text += ' Track ' + arrivalTrack;
          } else {
            text += '&nbsp;';
          }
        }
        if (showArrival && showDeparture) {
          text += '<br/>';
        }
        if (showDeparture) {
          if (departureTrack) {
            text += ' Track ' + departureTrack;
          } else {
            text += '&nbsp;';
          }
        }
        addLabel(text, 195, len * pos, {
          'font-size': '75%',
          'color': '#555',
        }, '65px', '2.5em');
      }.bind(this);

      const addDirectionLabel = function(train, direction, pos, expandable) {
        let text = '';
        if (expandable) {
          text += '<div style="line-height: 1.4; vertical-align: middle; display: table-cell; font-size: inherit; font-weight: bold" class="' + materialicons + '">&#xe5cc;</div>';
        }
        text += '<div style="display: table-cell">';
        if (train) {
          text += '<span style="font-weight: bold; color: #444">' + train + '</span>';
        }
        if (direction) {
          text += ' towards ';
          text += '<span style="font-weight: normal">' + direction + '</span>';
        }
        text += '</div>';
        addLabel(text, 195, len * (pos + 0.5), {
          'font-size': '78%',
          'font-weight': 'lighter',
        }, '220px', '2em');
      }.bind(this);


      let lastIsIcon = false;
      const rotateGroup = this.put(new SVG.G).move(180, radius).rotate(90);
      elements.forEach((el, i) => {
        let label = el.label;
        let isIcon = false;
        if (!el.label) {
          label = '\uE536';
          isIcon = true;
        }
        const move = new SVG.MotisMove;
        const moveGroup = rotateGroup.put(move)
                            .draw(thickness, radius, len, label, -90, isIcon)
                            .move(len * i, 0)
                            .fill(el.color);
        rotateGroup.add(moveGroup);

        addStationLabel(el.from.stop.name, i);
        addTimeLabel(i, i != 0 ? el.from.stop.arrival.time : null, el.from.stop.departure.time);
        addTrackLabel(i, i != 0 && !lastIsIcon, el.from.stop.arrival.platform, !isIcon, el.from.stop.departure.platform);
        const isExpandable = el.transport.move.range.from + 1 !== el.transport.move.range.to;
        addDirectionLabel(el.transport.move.name, el.transport.move.direction, i, isExpandable);

        lastIsIcon = isIcon;
      });

      const finalStopEl = elements[elements.length - 1];
      addStationLabel(finalStopEl.to.stop.name, elements.length);
      addTimeLabel(elements.length, finalStopEl.to.stop.arrival.time);
      addTrackLabel(elements.length, true, finalStopEl.to.stop.arrival.platform, false, finalStopEl.to.stop.departure.platform);

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
    const height = Math.max(this.segments.length * (this.movelen + 10) + 20, 200);
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
      if (i >= importantStops.length - 2) {
        return acc;
      }

      const to = importantStops[i + 1];
      const transport = transports(c, from.i, to.i)[0];
      if (transport) {
        acc.push({
          transport,
          from,
          to,
          label: transport.move.category_name,
          color: transport.move_type === 'Walk' ? colors.walk : colors[transport.move.clasz] || colors.fallback,
          begin: new Date(from.stop.departure.time * 1000),
          end: new Date(to.stop.arrival.time * 1000)
        });
      }
      return acc;
    }, []);
  }


  render() {
    return (
      <div style={{marginTop: '20px', height: Math.min(300, (this.state.height + 20)) + 'px', overflow: 'auto'}} {...this.props}>
        <div>
          <svg id="detailview" style={{height: this.state.height + 'px'}}></svg>
        </div>
      </div>
    );
  }
}
