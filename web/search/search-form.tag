<search-form>
  <div>
    <div class="pure-g gutters">
      <gb-typeahead name="from"
                    value={state().from}
                    data-src={guessStation}
                    display-property="name"
                    icon="&#xE8B4;"
                    class="pure-u-1 pure-u-sm-3-5"
                    placeholder={translate('From')}
                    label={translate('From')}
                    input-onchange={onChange('from')}>
        <gb-button class={'swap-locations-btn': true,
                          'flipped': parent.parent.locationsSwapped}
                   color="PRIMARY_COLOR"
                   btn-onclick={parent.parent.swapLocations}
                   circle outline size="small">
          <i class="icon">&#xE8D5;</i>
        </gb-button>
      </gb-typeahead>
      <div class="pure-u-1 pure-u-sm-2-5">
        <gb-taglist name="transports.to"
                    label={translate('Transports at the Start')}
                    tags={tags}
                    available-tags={availableTags}
                    active-tags={state().transports.from}
                    onremove-tag={onRemoveTransport('from')}
                    onadd-tag={onAddTransport('from')} />
      </div>
    </div>
    <div class="pure-g gutters">
      <gb-typeahead name="to"
                    value={state().to}
                    data-src={guessStation}
                    display-property="name"
                    icon="&#xE8B4;"
                    class="pure-u-1 pure-u-sm-3-5"
                    placeholder={translate('To')}
                    label={translate('To')}
                    input-onchange={onChange('to')}/>
      <div class="pure-u-1 pure-u-sm-2-5">
        <gb-taglist name="transports.from"
                    label={translate('Transports at the Destination')}
                    tags={tags}
                    available-tags={availableTags}
                    active-tags={state().transports.to}
                    onremove-tag={onRemoveTransport('to')}
                    onadd-tag={onAddTransport('to')} />
      </div>
    </div>
    <div class="pure-g gutters">
      <gb-date name="date"
               value={state().date}
               input-onchange={onChange('date')}
               class="pure-u-1 pure-u-sm-1-2"
               label={translate('Date')}/>
      <gb-time name="time"
               value={state().time}
               input-onchange={onChange('time')}
               class="pure-u-1 pure-u-sm-1-2"
               label={translate('Time')}/>
    </div>
  </div>

  <style type="text/precss">
  .search-btn {
    float: right;
  }

  .search-btn .icon {
    position: relative;
    top: 7px;
  }

  .swap-locations-btn {
    position: relative;
    right: 20px;
    top: 35px;
  }

  .swap-locations-btn .icon {
    font-size: 22px !important;
    transition: all 0.2s ease;
  }

  .flipped .icon { transform: rotate(180deg); }

  /* ---------- */
  .time-option {
    height: 50px;
    line-height: 20px;
  }

  .time-option { margin-top: 15px; }

  .time-option label {
    float: right;
    font-size: 70%;
    border-width: 0;
    border: 2px solid precss.LIGHTEN("PRIMARY_COLOR", 20);
    border-radius: .35em;
    cursor: pointer;
    text-align: center;
    margin-top: 2px;
    margin-bottom: 2px;
    padding: 2px 0 !important;
    height: auto !important;
    width: 100%;
  }

  .time-option label .icon {
    color: precss.LIGHTEN("PRIMARY_COLOR", 20);
    font-size: 120%;
    position: relative;
    top: 2px;
    cursor: pointer;
  }

  .time-option input:checked+label {
    background-color: precss.LIGHTEN("PRIMARY_COLOR", 20);
    color: white;
  }

  .time-option input +label ::before { content: "\E836"; }
  .time-option input:checked +label ::before { content: "\E837"; }
  </style>

  <script type="module">
  import server from 'util/server.js';
  import actions from 'redux/action-creators.js';

  this.mixin('i18n');

  this.store = this.opts.store
  this.state = () => this.opts.store.getState().search.query
  this.currState = this.state()

  this.activeTags = [];
  this.availableTags = [
    { icon: '\uE531', id: 'car' },
    { icon: '\uE536', id: 'walk' },
    { icon: '\uE52F', id: 'bike' }
  ];

  this.guessStation = input => new Promise(resolve => {
    server.send({
      destination: { type: 'Module', target: '/guesser' },
      content_type: 'StationGuesserRequest',
      content: { input, guess_count: 6 }
    })
    .then(response => resolve(response.content.guesses))
    .catch(err => console.error('station guess not possible: ', err));
  })

  this.onChange = key => value => this.store.dispatch(actions.updateSearchForm(key, value))
  this.onRemoveTransport = key => tag => this.store.dispatch(actions.searchRemoveTransportMode(key, tag))
  this.onAddTransport = key => tag => this.store.dispatch(actions.searchAddTransportMode(key, tag))
  this.swapLocations = () => {
    this.locationsSwapped = !this.locationsSwapped
    this.store.dispatch(actions.swapLocations())
  }

  this.store.subscribe(() => this.update())
  </script>
</search-form>
