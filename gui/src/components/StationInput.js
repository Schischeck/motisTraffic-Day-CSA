import React from 'react';
import Component from './BaseComponent';
import {Container} from 'flux/utils';

import Store from '../flux-infra/Store';
import Actions from '../flux-infra/Actions';
import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';

export class StationInputUI extends Component {
  render() {
    return (
      <div>
        <label htmlFor={ this.props.name }>{ this.props.label }</label>
        <input type="text" name={ this.props.name } onKeyUp={ this.props.onKeyUp } ></input>
        <ul className="suggestionbox">
          {
            this.props.suggestions.map((val, index) => {
              return <li key={ index }>{ val.name }</li>
            })
          }
        </ul>
      </div>
    );
  }
}

export class StationInput extends Component {
  constructor(props) {
    super(props);
  }

  static getStores() {
    return [Store];
  }

  static calculateState(prevState) {
    return Store.getState().toJS();
  }

  fetchSuggestions(evt) {
    let data = evt.target.value;
    Actions.sendMessage(new StationGuesserRequest(data, 5), this.componentId);
  }

  render() {
    let componentStatus = this.state[this.componentId];
    let guesses = componentStatus != undefined ? componentStatus.guesses : [];
    return (
      <StationInputUI
        {...this.props}
        onKeyUp = { this.fetchSuggestions.bind(this) }
        suggestions={ guesses } />
    );
  }
}

const container = Container.create(StationInput);
export default container;
