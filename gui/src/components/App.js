import React, { Component } from 'react';

import Counter from './Counter';
import DecreaseButton from './DecreaseButton';
import { NICE, SUPER_NICE } from '../Constants';

export class App extends Component {
  render() {
    return (
      <div>
        <Counter color={SUPER_NICE} />
        <DecreaseButton />
      </div>
    );
  }
}
