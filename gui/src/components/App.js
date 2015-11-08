import React, { Component } from 'react';
import ReactDOM from 'react-dom';

import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';

import Typeahead from './Typeahead';

export class App extends Component {
  guessStation(input) {
    return new Promise((resolve, reject) => {
      Server.sendMessage(new StationGuesserRequest(input, 7)).then(response => {
        resolve(response.content.guesses);
      });
    });
  }

  render() {
    return (
    <div>
      <Typeahead
                 name="From"
                 complete={ this.guessStation.bind(this) } />
      <Typeahead
                 name="To"
                 complete={ this.guessStation.bind(this) } />
    </div>
    );
  }
}
