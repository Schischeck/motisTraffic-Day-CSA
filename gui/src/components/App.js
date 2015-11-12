import React, { Component } from 'react';
import ReactDOM from 'react-dom';

import { AppBar, IconButton, Paper } from 'material-ui';

import PaddedPaper from './PaddedPaper';
import RoutingForm from './RoutingForm';
import Map from './Map';

import style from './App.scss';

export class App extends Component {
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
          <RoutingForm />
        </PaddedPaper>
      </div>
    </div>
    );
  }
}
