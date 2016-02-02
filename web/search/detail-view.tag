<detail-view>
  <gb-button class="btn-back" btn-onclick={opts.showOverview} size="small" outline>
    <span><i18n>back to overview</i18n></span>
  </gb-button>
  <div class="details">
    <div class="details-container"
         style={'height: ' + height + 'px'}
         name="container">
      <svg id="datailview"></svg>
    </div>
  </div>

  <style type="text/precss">
  .details-container { height: 100%; }

  .details {
    width: 100%;
    height: calc(100% - 3em);
    overflow-y: scroll;
  }

  .btn-back a {
    margin-bottom: 10px;
  }
  </style>

  <script type="module">
  import actions from 'redux/action-creators.js';
  import c from 'search/timeline/constants.js';
  import readConnections from 'search/timeline/read-connections.js';

  this.fixGoogleChrome = () => setInterval(() => this.update(), 100) // :-(

  this.drawDetailView = () => {
    if (this.opts.connection) {
      const segments = readConnections([this.opts.connection], false)[0];
      this.height = segments.length * c.DETAIL_VIEW_CONNECTION_LENGTH + c.RADIUS * 2 + 10;
      this.svg.clear();
      this.svg.detailView(segments).move(150);
    }
  }

  this.on('mount', () => {
    this.svg = SVG('datailview');
    this.on('update', this.drawDetailView);
    this.update();
    this.fixGoogleChrome();
  });
  </script>
</detail-view>
