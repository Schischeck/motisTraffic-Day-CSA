import React, { Component } from 'react';
import ReactDOM from 'react-dom';

import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';

import { Paper, AppBar, IconButton, NavigationClose } from 'material-ui/lib';
import Typeahead from './Typeahead';
import Map from './Map';

import style from './App.scss';

export class App extends Component {
  guessStation(input) {
    return new Promise((resolve, reject) => {
      Server.sendMessage(new StationGuesserRequest(input, 7)).then(response => {
        resolve(response.content.guesses);
      });
    });
  }

  render() {
    let appIcon = <IconButton iconClassName="material-icons">card_travel</IconButton>
  
    return (
      <div>
        <div className={style.overlay} style={{'width': '100%', 'height': '100%'}}>
          <Map></Map>
        </div>
        <div className={style.overlay} style={{'width': '100%'}}>
          <AppBar
            title="TD GUI"
            iconElementLeft={appIcon}
            style={{'width': '100%'}} />
          <Paper zDepth={1} style={ { padding: '1em', position: 'absolute', top: '100px', left: '100px', zIndex: 1 } }>
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
      </div>
    );
  }
}
