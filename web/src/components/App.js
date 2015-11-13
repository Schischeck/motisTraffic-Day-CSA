import React, { Component } from 'react';

import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';

import { Paper } from 'material-ui/lib';
import Typeahead from './Typeahead';
import Map from './Map';

export class App extends Component {
  guessStation(input) {
    return new Promise((resolve) => {
      Server.sendMessage(new StationGuesserRequest(input, 7)).then(response => {
        resolve(response.content.guesses);
      });
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
        </div>
      </Paper>
    </div>
    );
  }
}
