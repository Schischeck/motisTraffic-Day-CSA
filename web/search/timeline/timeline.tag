<timeline>
  <div class="time-axis-labels-container">
    <svg id="time-axis-labels"></svg>
  </div>
  <div class="connections">
    <div class="connections-container"
         style={'height: ' + getHeight() + 'px'}
         name="container">
      <svg id="connections"></svg>
    </div>
  </div>

  <style type="text/precss">
  .time-axis-labels-container { height: 15px; }

  .connections-container { height: 100%; }

  .connections {
    width: 100%;
    height: calc(100% - 15px);
    overflow-y: scroll;
  }
  </style>

  <script type="module">
  import calculateTimelineSettings from 'search/timeline/scale.js';
  import readConnections from 'search/timeline/read-connections.js';

  this.getHeight = () => Math.max(230, this.opts.connections.length * 72.5) + 50

  this.drawTimeline = () => {
    this.timeAxisLabels.clear();
    this.connections.clear();

    const cons = readConnections(this.opts.connections, true);
    const conSelect = id => safeCall(this.opts.connectionSelected, id)
    this.timelineScale = calculateTimelineSettings(cons, this.width, 50);
    this.timeAxisLabels.timeAxisLabels(this.timelineScale);
    this.connections.connectionGrid(this.width, this.getHeight(), this.timelineScale, cons, conSelect);
  }

  this.on('mount', () => {
    this.width = this.container.clientWidth;
    this.timeAxisLabels = SVG('time-axis-labels');
    this.connections = SVG('connections');
    this.on('update', this.drawTimeline);
    this.update();
  });
  </script>
</timeline>
