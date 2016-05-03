import c from 'search/timeline/constants.js';

SVG.DetailView = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    draw: function(elements) {
      const formatTimestamp = ts => formatTime(new Date(ts * 1000))

      const addLabel = (text, x, y, style, width, height) => {
        width = width || '100%';
        height = height || '100%';
        style['width'] = width;
        style['height'] = height;
        style['display'] = 'table-cell';
        style['vertical-align'] = 'middle';

        const styleStr = Object.keys(style).map(key => key + ':' + style[key] + ';').join('');

        const container = document.createElement('div');
        container.innerHTML = text;

        const svgForeignObj = this.put(new SVG.ForeignObject).size(width, height);
        svgForeignObj.appendChild(container, {style: styleStr});
        svgForeignObj.move(x, y + 5);
        this.add(svgForeignObj);
      }

      const addTimeLabel = (pos, showArrival, arrivalTime, showDeparture, departureTime) => {
        var text = '';
        text += showArrival && arrivalTime ? formatTimestamp(arrivalTime) : '&nbsp;';
        text += showArrival && showDeparture ? '<br/>' : '';
        text += showDeparture ? formatTimestamp(departureTime) : '&nbsp;';
        addLabel(text, 0, c.DETAIL_VIEW_CONNECTION_LENGTH * pos - c.RADIUS / 2, {
          'font-size': '75%',
          'color': '#555'
        }, '40px', '2.5em');
      };

      const addStationLabel = (text, pos) => {
        addLabel(text, 145, c.DETAIL_VIEW_CONNECTION_LENGTH * pos - c.RADIUS / 1.5, {
          'font-size': '90%',
          'font-weight': 'lighter'
        }, '220px', '2.5em');
      };

      const addTrackLabel = (pos, showArrival, arrivalTrack, showDeparture, departureTrack) => {
        var text = '';
        if (showArrival) {
          if (arrivalTrack) {
            text += 'Track ' + arrivalTrack;
          } else {
            text += '&nbsp;';
          }
        }
        if (showArrival && showDeparture) {
          text += '<br/>';
        }
        if (showDeparture) {
          if (departureTrack) {
            text += 'Track ' + departureTrack;
          } else {
            text += '&nbsp;';
          }
        }
        addLabel(text, 85, c.DETAIL_VIEW_CONNECTION_LENGTH * pos - c.RADIUS / 2, {
          'font-size': '75%',
          'color': '#555',
        }, '65px', '2.5em');
      };

      const addDirectionLabel = (train, direction, pos, expandable) => {
        var text = '';
        if (expandable) {
          text += '<div style="vertical-align: middle; display: table-cell; font-size: .9em; font-weight: bold" class="icon">&#xe5cc;</div>';
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
        addLabel(text, 80, c.DETAIL_VIEW_CONNECTION_LENGTH * (pos + 0.5), {
          'font-size': '78%',
          'font-weight': 'lighter',
        }, '220px', '2em');
      };

      const rotateGroup = this.put(new SVG.G).move(65, c.RADIUS).rotate(90);

      var lastIsIcon = false;
      elements.forEach((el, i) => {
        const isIcon = !el.transport.move.category_name;
        const label = isIcon ? '\uE536' : el.transport.move.category_name;

        const moveGroup = rotateGroup.put(new SVG.Move)
                                     .draw(c.DETAIL_VIEW_CONNECTION_LENGTH - c.CIRCLE_WIDTH, label, el.color, -90, isIcon)
                                     .fill(el.color)
                                     .move(i * c.DETAIL_VIEW_CONNECTION_LENGTH, 0);
        addStationLabel(el.from.stop.station.name, i);
        addTimeLabel(i, i != 0 && !lastIsIcon, el.from.stop.arrival.time, !isIcon, el.from.stop.departure.time);
        addTrackLabel(i, i != 0 && !lastIsIcon, el.from.stop.arrival.platform, !isIcon, el.from.stop.departure.platform);
        const isExpandable = el.transport.move.range.from + 1 !== el.transport.move.range.to;
        addDirectionLabel(el.transport.move.name, el.transport.move.direction, i, isExpandable);
        lastIsIcon = isIcon;
      });

      const arrivalCircleX = elements.length * c.DETAIL_VIEW_CONNECTION_LENGTH;
      rotateGroup.put(new SVG.Circle)
                 .move(arrivalCircleX, c.THICKNESS / 2)
                 .fill('#666')
                 .radius(c.RADIUS);

      const finalStopEl = elements[elements.length - 1];
      addStationLabel(finalStopEl.to.stop.station.name, elements.length);
      addTimeLabel(elements.length, true, finalStopEl.to.stop.arrival.time);
      addTrackLabel(elements.length,
                    true, finalStopEl.to.stop.arrival.platform,
                    false, finalStopEl.to.stop.departure.platform);

      this.add(rotateGroup);
      return this;
    }
  },
  construct: {
    detailView: function(elements, hoverBegin, hoverEnd) {
      return this.put(new SVG.DetailView)
                 .draw(elements, hoverBegin, hoverEnd);
    }
  }
});
