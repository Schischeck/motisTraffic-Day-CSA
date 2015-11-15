import React, { Component } from 'react';
import ReactDOM from 'react-dom';

import { AppBar, IconButton, Paper, RaisedButton } from 'material-ui';

import PaddedPaper from './PaddedPaper';
import RoutingForm from './RoutingForm';
import Map from './Map';
import ConnectionView from './ConnectionView';
import Typeahead from './Typeahead';

import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';
import RoutingRequest from '../Messages/RoutingRequest';

import style from './App.scss';

export class App extends Component {
  constructor(props) {
    super(props);
    this.state = {'connections': [], 'showError': false};
  }

  guessStation(input) {
    return new Promise(resolve => {
      Server.sendMessage(new StationGuesserRequest(input, 7)).then(response => {
        resolve(response.content.guesses);
      });
    });
  }

  getRouting() {
    Server.sendMessage(new RoutingRequest()).then(response => {
      this.setState({'connections': response.content.connections, 'showError': false});
    }).catch(error => {
      this.setState({'connections': [], 'showError': true});
    });
  }

  render() {
    let appIcon = <IconButton iconClassName="material-icons">
                    card_travel
                  </IconButton>
    return (
    <div>
      <div
           className={ style.overlay }
           style={ {  'width': '100%',  'height': '100%'} }>
        <Map></Map>
      </div>
      <div
           className={ style.overlay }
           style={ {  'width': '100%'} }>
        <AppBar
                title="TD GUI"
                iconElementLeft={ appIcon }
                style={ {  'width': '100%'} } />
        <PaddedPaper
                     zDepth={ 1 }
                     style={ {  position: 'fixed',  'height': '100%',  left: '0',  zIndex: 1} }>
          <RoutingForm
                       onRequestRouting={ this.getRouting.bind(this) } />
          <ConnectionView
                   connections={ this.state.connections }
                   showError={ this.state.showError } />
        </PaddedPaper>
      </div>
    </div>
    );
  }
}
