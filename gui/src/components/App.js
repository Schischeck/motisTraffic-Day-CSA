import React, { Component } from 'react';
import ReactDOM from 'react-dom';

import StationInput from './StationInput';

export class App extends Component {
  render() {
    return (
      <div>
        <StationInput name="station1" />
      </div>
    );
  }
}
