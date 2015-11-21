import React, { Component } from 'react';

import { AppBar, IconButton } from 'material-ui';

import PaddedPaper from './PaddedPaper';
import RoutingForm from './RoutingForm';
import Map from './Map';
import ConnectionView from './ConnectionView';

import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';
import RoutingRequest from '../Messages/RoutingRequest';

import style from './App.scss';

export class App extends Component {
  constructor(props) {
    super(props);
    this.state = {
      'connections': [],
      'showError': false,
      'waiting': false,
      'showResults': false
    };
  }

  getRouting() {
    this.setState({
      'waiting': true,
    });

    this.refs.routingform.getData().then(parameters => {
      const unixTime = Math.floor(parameters.date / 1000);
      return new RoutingRequest(unixTime, [
        parameters.from,
        parameters.to
      ]);
    }).then(request => {
      return Server.sendMessage(request);
    }).then(response => {
      this.setState({
        'connections': response.content.connections,
        'showError': false,
        'waiting': false,
        'showResults': true
      });
    }).catch(error => {
      console.error(error);
      this.setState({
        'connections': [],
        'showError': true,
        'waiting': false,
        'showResults': true
      });
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
                        card_travel
                      </IconButton> );

    const connectionView = this.state.showResults ?
      (<ConnectionView
                       connections={ this.state.connections }
                       showError={ this.state.showError }
                       waiting={ this.state.waiting } />) : <div />;
    return (
    <div>
      <div
           className={ style.overlay }
           style={ { 'width': '100%', 'height': '100%'} }>
        <Map />
      </div>
      <div
           className={ style.overlay }
           style={ { 'width': '100%'} }>
        <AppBar
                title="TD GUI"
                iconElementLeft={ appIcon }
                style={ { 'width': '100%'} } />
        <PaddedPaper
                     zDepth={ 1 }
                     style={ { position: 'fixed', 'height': '100%', left: '0', zIndex: 1} }>
          <RoutingForm
                       ref="routingform"
                       disabled={ this.state.waiting }
                       onRequestRouting={ this.getRouting.bind(this) } />
          { connectionView }
        </PaddedPaper>
      </div>
    </div>
    );
  }
}
