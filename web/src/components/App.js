import React, { Component } from 'react';

import { AppBar, IconButton, CircularProgress } from 'material-ui';

import PaddedPaper from './PaddedPaper';
import RoutingForm from './RoutingForm';
import Map from './Map';
import ConnectionView from './ConnectionView';
import ServerConnectionIndicator from './ServerConnectionIndicator';
import Timeline from './timeline/MotisTimeline';
import DetailView from './timeline/MotisDetailView';

import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';

import style from './App.scss';

export class App extends Component {
  constructor(props) {
    super(props);
    const cached = JSON.parse(localStorage.getItem('motiscache')) || {};
    const lastReq = cached.req;
    let lastFrom = lastReq ? lastReq.content.path[0].name : '';
    let lastTo = lastReq ? lastReq.content.path[1].name : '';
    let lastDate = lastReq ? new Date(lastReq.content.interval.begin * 1000) : new Date();
    this.state = {
      'connections': cached.connections || [],
      'lastFrom': lastFrom,
      'lastTo': lastTo,
      'lastDate': lastDate,
      'showError': false,
      'waiting': false,
      'showResults': !!cached.connections
    };
  }

  getRouting() {
    this.setState({
      'waiting': true,
    });

    const req = this.refs.routingform.getRequest();
    Server.sendMessage(req).then(response => {
      this.setState({
        'connections': response.content.connections,
        'showError': false,
        'waiting': false,
        'showResults': true
      });
      localStorage.setItem('motiscache', JSON.stringify({
        'connections': response.content.connections,
        'req': req
      }));
    });
  }

  guessStation(input) {
    return new Promise(resolve => {
      Server.sendMessage(new StationGuesserRequest(input, 7)).then(response => {
        resolve(response.content.guesses);
      });
    });
  }

  render() {
    const appIcon = ( <IconButton iconClassName="material-icons">
                        <span style={{'position': 'relative', 'top': '-0.24em', 'left': '-1px' }}>
                          <svg version="1.1" width="1.3em" height="1.5em" viewBox="729.3 444.505 100.006 100.006" enable-background="new 729.3 444.505 100.006 100.006">
                            <g fill="#444">
                              <path d="M729.872,487.327c-0.237,1.655-0.391,3.332-0.464,5.033h75.879c-0.265-0.518-0.621-0.985-0.98-1.442  c-12.972-16.769-19.95-15.315-29.932-15.741c-3.328-0.137-5.585-0.192-18.832-0.192c-7.09,0-14.798,0.018-22.304,0.038  c0.737-1.746,1.6-3.419,2.525-5.061c-2.885,5.106-4.865,10.771-5.805,16.789l0.891-4.397c0.007-0.031,0.018-0.063,0.024-0.094  h38.883v5.067H729.872z"></path>
                              <path d="M805.885,497.432h-76.438c0.08,1.352,0.206,2.686,0.388,4.002c29.299,0,66.89,0,70.571,0  C803.552,501.434,805.313,499.648,805.885,497.432z"></path>
                              <path d="M733.851,515.257c-0.74-1.624-1.404-3.286-1.98-4.997c6.608,19.89,25.328,34.251,47.433,34.251  c20.205,0,37.566-12.007,45.452-29.254H733.851z"></path>
                              <path d="M729.38,492.915c-0.018,0.531-0.08,1.055-0.08,1.589c0,0.538,0.063,1.059,0.08,1.59V492.915z"></path>
                              <path d="M824.77,515.229c0.736-1.614,1.4-3.266,1.963-4.967C826.17,511.96,825.506,513.612,824.77,515.229z"></path>
                              <path d="M779.303,444.505c-18.682,0-34.939,10.265-43.524,25.439c6.709-0.014,19.775-0.022,19.775-0.022h0.003  v-0.005c15.444,0,16.018,0.069,19.035,0.195l1.868,0.069c6.507,0.217,14.505,0.916,20.798,5.68  c3.416,2.584,8.348,8.287,11.288,12.35c2.718,3.758,3.5,8.078,1.652,12.217c-1.701,3.804-5.361,6.073-9.793,6.073h-69.555  l-0.884-4.201c0.426,2.707,1.037,5.344,1.879,7.886h94.914c1.631-4.935,2.546-10.198,2.546-15.682  C829.306,466.893,806.919,444.505,779.303,444.505z"></path>
                            </g>
                          </svg>
                        </span>
                      </IconButton> );

    const connectionView = this.state.showResults ?
      (<ConnectionView
                       connections={ this.state.connections }
                       showError={ this.state.showError }
                       waiting={ this.state.waiting } />) : <div />;

    const results = this.state.waiting
                    ? <CircularProgress
                          mode="indeterminate"
                          style={ {  'margin': '50px auto',  'display': 'block'} } />
                    : <Timeline connections={ this.state.connections } />;

    return (
    <div className={ style.app }>
      <Map />
      <PaddedPaper
        className={ style.overlay }
        zDepth={ 1 }>
          <div className={ style.topbar }>
            <ServerConnectionIndicator iconStyle={{ 'color': '#999' }}/>
          </div>
          <RoutingForm
                       ref="routingform"
                       initFrom={ this.state.lastFrom }
                       initTo={ this.state.lastTo }
                       initDate={ this.state.lastDate }
                       disabled={ this.state.waiting }
                       onRequestRouting={ this.getRouting.bind(this) } />
          { results }
          // Detail View with mock-data
          /*
          <DetailView connection={[
            {
              "name": "Frankfurt (Main) Hbf",
              "color":"#19DD89",
              "label":"RE"
            },
            {
              "name": "Hanau Hbf",
              "color":"#D31996",
              "label":"ICE"
            },
            {
              "name": "Sonstwo",
              "color":"#FD8F3A",
              "label":"RB"
            },
            {
              "name": "In der Wallachei",
              "color":"#FD8F3A",
              "label":"RB"
            },
            {
              "name": "Lorem Ipsum",
              "color":"#19DD89",
              "label":"RE"
            },
            {
              "name": "Berlin Hbf",
              "color":"#19DD89",
              "label":"RE"
            }
          ]} />
          */
      </PaddedPaper>
    </div>
    );
  }
}
