import c from 'search/timeline/constants.js';

SVG.InfoHover = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    moveInner: function(x, y) { this.hover.move(x, y); },

    updateContent: function(el) {
      const from = el.from.stop;
      const to = el.to.stop;
      const transport = el.transport.move;

      const departure = {
        time: formatTime(new Date(from.departure.time * 1000)),
        isDelayed: from.departure.time !== from.departure.schedule_time,
        track: from.departure.platform
      };

      const arrival = {
        time: formatTime(new Date(to.arrival.time * 1000)),
        isDelayed: to.arrival.time !== to.arrival.schedule_time,
        track: to.arrival.platform
      };

      this.content.innerHTML =
        '<table style="padding: 3px 5px; margin: 0">' +
          '<tr>' +
            '<td>' + from.name + '</td>' +
            '<td>' +
              '<span style="' + (departure.isDelayed ? 'color: #A33' : '') + '">' +
                departure.time +
              '</span>' +
            '</td>' +
            '<td style="min-width: 45px; border-left: dashed 1px #CCC; margin-left: 8px; padding-left: 8px; text-align: right" rowspan="2">' +
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
          '</tr>' +
         '</table>';
    },

    style: function() {
      var style = '';
      style += 'margin: 20px;';
      style += 'max-height: 80px;';
      style += 'width: 240px;';
      style += 'display: none;';
      style += 'box-shadow: 0 3px 14px rgba(0,0,0,0.4);';
      style += 'color: #777;';
      style += 'font-family: sans-serif;';
      style += 'font-size: .7em;';
      style += 'border-radius: 2px;';
      style += 'background-color: white;';
      style += 'border: 1px solid #BBB;';
      return style;
    },

    show: function() {
      this.content.style.display = 'table';
      this.content.style.visibility = 'visible';
    },

    hide: function() {
      this.content.style.display = 'none';
      this.content.style.visibility = 'hidden';
      this.hover.move(1000, 1000);
    },

    draw: function() {
      const g = this.put(new SVG.G);

      this.hover = this.put(new SVG.ForeignObject);
      this.hover.size(c.INFO_HOVER_WIDTH, c.INFO_HOVER_HEIGHT);

      this.content = document.createElement('div');
      this.hover.appendChild(this.content, {style: this.style()});

      g.add(this.hover);

      this.hide();

      return g;
    }
  }
});