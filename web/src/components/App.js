import React, { Component } from 'react';

import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';
import RoutingRequest from '../Messages/RoutingRequest';

import ConnectionView from './ConnectionView';

import { Paper, RaisedButton } from 'material-ui/lib';
import Typeahead from './Typeahead';
import Map from './Map';

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
    return (
    <div>
      <Map />
      <Paper
             zDepth={ 1 }
             style={ { padding: '1em', position: 'absolute', top: '100px', left: '100px', zIndex: 1} }>
        <div>
          <Typeahead
                     name="From"
                     complete={ this.guessStation.bind(this) } />
          <Typeahead
                     name="To"
                     complete={ this.guessStation.bind(this) } />
          <RaisedButton
                       style={{'float': 'right'}}
                       label="Find Route"
                       primary={ true }
                       onClick={this.getRouting.bind(this)} />
          <ConnectionView
                   connections={ this.state.connections }
                   showError={ this.state.showError } />
        </div>
      </Paper>
    </div>
    );
  }
}
