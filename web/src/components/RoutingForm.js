import React, { Component } from 'react';

import { DatePicker, FloatingActionButton, TimePicker, RaisedButton, ClearFix, RadioButton, RadioButtonGroup } from 'material-ui';

import Server from '../Server';
import RoutingRequest from '../Messages/RoutingRequest';
import StationGuesserRequest from '../Messages/StationGuesserRequest';
import Typeahead from './Typeahead';

import style from './RoutingForm.scss';
import iconcss from './MaterialIcons.scss';
const materialicons = iconcss['material-icons'];

export default class RoutingForm extends Component {
  constructor(props) {
    super(props);
    this.state = {
      time: props.initDate || new Date()
    };
  }

  componentDidMount = this.componentDidUpdate

  componentDidUpdate() {
    // Update TimePicker by hand
    // https://github.com/callemall/material-ui/issues/1710
    this.refs.timePicker.setTime(this.state.time);
  }

  // First argument is always null
  onTimeChange(ignore, date) {
    const newTime = this.state.time;
    newTime.setMinutes(date.getMinutes());
    newTime.setHours(date.getHours());
    this.setState({
      'time': newTime
    });
  }

  // First argument is always null
  onDateChange(ignore, date) {
    const newTime = new Date(date.getFullYear(),
      date.getMonth(),
      date.getDate(),
      this.state.time.getHours(),
      this.state.time.getMinutes(), 0, 0, 0);
    this.setState({
      'time': newTime
    });
  }

  getRequest() {
    const fromValue = this.refs.fromInput.getValue();
    const toValue = this.refs.toInput.getValue();
    const fromHasEva = fromValue.eva !== undefined;
    const toHasEva = toValue.eva !== undefined;
    return new RoutingRequest(Math.floor(this.state.time / 1000), [
      {
        'name': fromValue.name,
        'eva_nr': fromHasEva ? fromValue.eva : ''
      },
      {
        'name': toValue.name,
        'eva_nr': toHasEva ? toValue.eva : ''
      }
    ]);
  }

  guessStation(input) {
    return new Promise((resolve) => {
      Server.sendMessage(new StationGuesserRequest(input)).then(response => {
        resolve(response.content.guesses);
      });
    });
  }

  switchStations() {
    const fromState = this.refs.fromInput.state;
    const toState = this.refs.toInput.state;
    this.refs.toInput.setState(fromState);
    this.refs.fromInput.setState(toState);
  }

  render() {
    return (
    <div>
      <FloatingActionButton
                            onClick={ this.switchStations.bind(this) }
                            mini={ true }
                            secondary={ true }
                            style={ { 'position': 'absolute', 'margin-top': '80px', 'left': 'calc(50% - 50px)', 'zIndex': 1, 'transform': 'scale(.8)'} }>
        <i className={ materialicons }>&#xE8D5;</i>
      </FloatingActionButton>
      <FloatingActionButton
                            onClick={ this.props.onRequestRouting }
                            secondary={ true }
                            style={ { 'position': 'absolute', 'marginTop': '110px', 'left': 'calc(50% + 255px)', 'zIndex': 1 } }>
        <i className={ materialicons }>search</i>
      </FloatingActionButton>
      <div className={ style.flexrow }>
        <div className={ style.flexcol }>
          <Typeahead
                     ref="fromInput"
                     hintText="From"
                     initVal={this.props.initFrom}
                     complete={ this.guessStation.bind(this) } />
          <Typeahead
                     ref="toInput"
                     hintText="To"
                     initVal={this.props.initTo}
                     complete={ this.guessStation.bind(this) } />
        </div>
        <div className={ style.flexcol }>
          <DatePicker
                      floatingLabelText="Day"
                      DateTimeFormat={ Intl.DateTimeFormat }
                      locale={ 'de' }
                      value={ this.state.time }
                      onChange={ this.onDateChange.bind(this) } />
          <TimePicker
                      ref="timePicker"
                      format="24hr"
                      floatingLabelText="Time"
                      container="inline"
                      onChange={ this.onTimeChange.bind(this) } />
        </div>
      </div>
      <ClearFix />
    </div>
    );
  }
}
