import React, { Component } from 'react';

import { DatePicker, FloatingActionButton, TimePicker, RaisedButton, ClearFix } from 'material-ui';

import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';
import Typeahead from './Typeahead';

export default class RoutingForm extends Component {
  constructor(props) {
    super(props);
    this.state = {
      time: new Date()
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

  switchStations() {
    const fromValue = this.refs.fromInput.getValue();
    const toValue = this.refs.toInput.getValue();
    this.refs.fromInput.setValue(toValue);
    this.refs.toInput.setValue(fromValue);
  }

  guessStation(input) {
    return new Promise((resolve) => {
      Server.sendMessage(new StationGuesserRequest(input, 7)).then(response => {
        resolve(response.content.guesses);
      });
    });
  }

  render() {
    const now = new Date();
    const in8Weeks = new Date();
    in8Weeks.setDate(now.getDate() + (8 * 7));

    return (
    <div>
      <e>
        Routing
      </e>
      <Typeahead
                 ref="fromInput"
                 hintText="From"
                 complete={ this.guessStation.bind(this) } />
      <FloatingActionButton
                            onClick={ this.switchStations.bind(this) }
                            mini={ true }
                            secondary={ true }
                            style={ {  'position': 'absolute',  'left': '276px',  'marginTop': '5px'} }>
        <i className="material-icons">&#xE8D5;</i>
      </FloatingActionButton>
      <Typeahead
                 ref="toInput"
                 hintText="To"
                 complete={ this.guessStation.bind(this) } />
      <DatePicker
                  floatingLabelText="Day"
                  DateTimeFormat={ Intl.DateTimeFormat }
                  minDate={ now }
                  maxDate={ in8Weeks }
                  value={ this.state.time }
                  onChange={ this.onDateChange.bind(this) } />
      <TimePicker
                  ref="timePicker"
                  format="24hr"
                  floatingLabelText="Time"
                  onChange={ this.onTimeChange.bind(this) } />
      <RaisedButton
                    style={ {  'float': 'right'} }
                    label="Find Connections"
                    primary={ true }
                    onClick={ this.props.onRequestRouting } />
      <ClearFix />
    </div>
    );
  }
}
