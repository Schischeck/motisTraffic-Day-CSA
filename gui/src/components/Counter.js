import React, { Component } from 'react';
import {Container} from 'flux/utils';

import AppDispatcher from '../flux-infra/Dispatcher';
import AppStore from '../flux-infra/Store';
import Actions from '../flux-infra/Actions';

export class CounterUI extends Component {
  render() {
    return (
      <h1 style={{color: this.props.color}}>
        Counter {this.props.value}
      </h1>
    );
  }
}

export class Counter extends Component {
  constructor(props) {
    super(props);
    this.interval = setInterval(() => this.tick(), 1000);
  }

  static getStores() {
    return [AppStore];
  }

  static calculateState(prevState) {
    return AppStore.getState().toJS();
  }

  tick() {
    Actions.up();
  }

  componentWillUnmount() {
    clearInterval(this.interval);
  }

  render() {
    return (
      <CounterUI {...this.props} value={this.state.value} />
    );
  }
}

const container = Container.create(Counter);
export default container;
