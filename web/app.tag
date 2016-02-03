<app>
  <div class="app">
    <gb-map />
    <div class="overlay" name="overlay">
      <div id="header">
        <h1 class="disable-select">Motis Project</h1>
        <i class="icon">{isConnected ? '&#xe2bf;' : '&#xe2c1;'}</i>
      </div>
      <div id="search">
        <search store={opts.store}/>
      </div>
    </div>
  </div>

  <style type="text/precss">
  body {
    margin: 0;
    padding: 0;
    color: TEXT_COLOR;
  }

  html, button, input, select, textarea,
  .pure-g [class *= "pure-u"] {
      font-family: Helvetica, sans-serif;
  }
  </style>

  <style type="text/precss" scoped>
  #search, #header {
    padding: 16px;
  }

  #header {
    display: flex;
    justify-content: space-between;
    align-items: flex-end;
    background: #2f2e3e;
    color: white;
    font-weight: lighter;
    border-radius: 3px 3px 0 0;
    box-shadow: 0 2px 5px rgba(0,0,0,.16);
  }

  #header h1 {
    font-size: 1.2em;
    margin: 0;
    font-weight: 400;
    cursor: default;
  }

  .app {
    position: fixed;
    z-index: 3;
    width: 100%;
    height: 100%;
    display: flex;
    align-items: center;
    justify-content: center;
  }

  .overlay {
    position: relative;
    z-index: 10;
    background-color: white;
    box-shadow: 0 3px 9px rgba(0,0,0,.16);
    border-radius: 3px;
    transition: .4s ease all;
    width: 650px;
    height: 640px;
  }
  </style>

  <script type="module">
  import server from 'util/server.js';
  import actions from 'redux/action-creators.js';

  server.on('connected', () => this.update());
  this.on('update', () => this.isConnected = server.isConnected());

  this.tags['gb-map'].on('map-mounted', map => new RailViz(map))

  this.on('mount', () => {
    this.overlay.ondrop = e => {
      console.log('dropped!');
      var file = e.dataTransfer.files[0];
      var reader = new FileReader();
      reader.readAsText(file);
      reader.onload = ev => {
        const dropResponse = JSON.parse(ev.target.result)
        const currQuery = JSON.parse(JSON.stringify(this.opts.store.getState().search.query))
        this.opts.store.dispatch(actions.receiveRouting(currQuery, dropResponse))
      };
      e.preventDefault();
    };
    this.overlay.ondragenter = e => e.preventDefault()
    this.overlay.ondragover = e => e.preventDefault()
  });
  </script>
</app>
