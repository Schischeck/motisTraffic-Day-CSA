import React, { Component } from 'react';
import {Container} from 'flux/utils';

import AppDispatcher from '../flux-infra/Dispatcher';
import AppStore from '../flux-infra/Store';
import Actions from '../flux-infra/Actions';

export default class DecreaseButton extends Component {
  render() {
    return (
      <button {...this.props} onClick={Actions.down}>Decrease</button>
    );
  }
}
