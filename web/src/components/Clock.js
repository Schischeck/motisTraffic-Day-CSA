import React, { Component } from 'react';
import { Container } from 'flux/utils';
import { Paper } from 'material-ui';
import TimeStore from './railviz/TimeStore';
import Dispatcher from '../Dispatcher';

export default class Clock extends Component {
  constructor(props) {
    super(props);
  }


  static getStores() {
    return [
      TimeStore
    ];
  }

  static calculateState(/* prevState */) {
    return TimeStore.getState();
  }

  componentDidMount() {
    this.interval = setInterval(this._tick, 1000);
  }

  _tick() {
    Dispatcher.dispatch({
      content_type: 'tick'
    });
  }

  getDate() {
    const time = TimeStore.getTime();
    const date = new Date(time);
    const minutes = date.getMinutes() < 10 ? '0' + date.getMinutes() : date.getMinutes();
    const hours = date.getHours() < 10 ? '0' + date.getHours() : date.getHours();
    const seconds = (date.getSeconds() + 1) < 10 ? '0' + (date.getSeconds() + 1) : (date.getSeconds() + 1);
    const day = date.getDate() < 10 ? '0' + date.getDate() : date.getDate();
    const month = (date.getMonth() + 1) < 10 ? '0' + (date.getMonth() + 1) : (date.getMonth() + 1);
    return hours + ':' + minutes + ':' + seconds + ' ' + day + '/' + month + '/' + date.getFullYear();
  }

  render() {
    return ( < Paper style={ { right: '0', 'top': '0', padding: '15px'} }>
               { this.getDate() }
               < /Paper>
    );
  }
}

const container = Container.create(Clock);
export default container;
