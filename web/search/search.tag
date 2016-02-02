<search>
  <detail-view if={showDetailView}
               show-overview={showOverview}
               connection={state().result.response[state().result.selectedConnection]} />
  <search-form if={!showDetailView} store={store} />
  <search-results if={!showDetailView} store={store} />

  <script type="module">
  import actions from 'redux/action-creators.js';

  this.store = this.opts.store;
  this.state = () => this.store.getState().search

  this.showOverview = () => this.opts.store.dispatch(actions.selectConnection(undefined))

  this.opts.store.subscribe(() => {
    const nextShowDetailView = this.state().result.selectedConnection !== undefined;
    if (this.showDetailView !== nextShowDetailView) {
      this.showDetailView = nextShowDetailView;
      this.update();
    }
  })
  </script>
</search>