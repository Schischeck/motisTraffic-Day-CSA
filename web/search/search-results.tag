<search-results>
  <div class="results">
    <div class="loading" if={state().result.loading}>
      <gb-spinner />
    </div>

    <div if={!queryMatchesResponse &&
             !state().result.loading &&
             !state().result.error}
         class="search-button-box">
      <gb-button btn-onclick={requestSearch}
                 color="PRIMARY_COLOR" size="large">
        <span>
          <i class="icon">&#xE8B6;</i>
          <i18n>Search</i18n>
        </span>
      </gb-button>
      <gb-button if={state().result.response}
                 btn-onclick={showLastResults}
                 class="show-last-btn"
                 color="PRIMARY_COLOR" size="medium" outline>
        <span>
          <i class="icon">&#xE15E;</i>
          <i18n>Show results of the last search</i18n>
        </span>
      </gb-button>
    </div>

    <div if={(state().result.error ||
              (queryMatchesResponse && state().result.response.length === 0)) &&
             !state().result.loading}>
      <div class="error icon"></div>
      <p style="text-align: center; color: #aaa; font-weight: bold">
        <i18n if={state().result.error}>An error occured.</i18n>
        <i18n if={!state().result.error}>No connections found.</i18n>
      </p>
    </div>

    <timeline if={queryMatchesResponse &&
                  !state().result.loading &&
                  state().result.response.length !== 0 &&
                  state().result.selected_connection === undefined}
              connections={state().result.response}
              connection-selected={connectionSelected}
              style="width: 100%; height: 100%" />
  </div>

  <style scoped>
  .show-last-btn { margin-top: 20px; }

  a .icon {
    position: relative;
    top: 5px;
  }

  .results {
    height: 300px !important;
    display: flex;
    align-items: center;
    justify-content: center;
    height: 100%;
  }

  .search-button-box {
    display: flex;
    flex-direction: column;
    width: 400px;
  }

  .loading {
    width: 100%;
    margin-top: 20px;
    padding: 20px;
    text-align: center;
  }

  .error:before {
    content: "\E001";
    width: 100%;
    height: 100%;
    font-size: 5.0em;
    text-align: center;
    display: block;
    line-height: 100px;
    color: #bbb;
  }

  .error {
    padding-top: 30px;
    width: 100%;
  }
  </style>

  <script type="module">
  import actions from 'redux/action-creators.js';

  this.store = this.opts.store;
  this.state = () => this.opts.store.getState().search

  this.connectionSelected = id => this.store.dispatch(actions.selectConnection(id));
  this.requestSearch = () => this.store.dispatch(actions.requestRouting)
  this.showLastResults = () => this.store.dispatch(actions.resetQueryToLast())

  this.on('update', () => {
    const query = JSON.stringify(this.state().query);
    const response = JSON.stringify(this.state().result.request);
    this.queryMatchesResponse = query === response;
  })

  this.store.subscribe(() => this.update())
  </script>
</search-results>
