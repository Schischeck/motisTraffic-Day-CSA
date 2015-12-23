import SVG from 'svg.js';

function pad(num, size) {
  const s = '000' + num;
  return s.substr(s.length - size);
}

SVG.TimeLabels = SVG.invent({
  create: 'g',
  inherit: SVG.G,
  extend: {
    drawTimeline: function(timelineSettings) {
      if (this.timelineElements) {
        this.timelineElements.forEach(c => { c.remove(); });
      }
      this.timelineElements = [];

      const t = new Date(timelineSettings.start.getTime());
      const totalCuts = timelineSettings.totalCuts;
      const scale = timelineSettings.scale;

      for (let cut = 0; cut < totalCuts - 1; cut++) {
        t.setTime(t.getTime() + scale);
        const x = timelineSettings.timeToXIntercept(t);
        const label = this.put(new SVG.Text)
                        .text(pad(t.getHours(), 2) + ':' + pad(t.getMinutes(), 2))
                        .attr({'font-family': 'Verdana,sans-serif'})
                        .attr({'text-anchor': 'middle'})
                        .size(9)
                        .move(x, 10);
        this.add(label);
        this.timelineElements.push(label);
      }
    }
  },

  construct: {
    motistimelabels: function() {
      return this.put(new SVG.TimeLabels);
    }
  }
});
