import React from 'react';
import Component from './BaseComponent';
import {Container} from 'flux/utils';

import {TextField, List, ListItem, Paper} from 'material-ui/lib';

import Menu from 'material-ui/lib/menus/menu';
import MenuItem from 'material-ui/lib/menus/menu-item';

import Store from '../flux-infra/Store';
import Actions from '../flux-infra/Actions';
import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';

import './StationInput.scss';

export class StationInputUI extends Component {
  render() {
    return (
      <div style={{'width': 256, 'position': 'relative'}}>
        <TextField
          hintText={ this.props.name }
          onKeyUp={ this.props.onKeyUp }
          onBlur={ this.onInputBlur }>
        </TextField>
        <Paper ref={'suggestionbox'} zDepth={1} className={['suggestions']}>
          <List desktop={true}>
            {
              this.props.suggestions.map((val, index) => {
                return <ListItem key={ index } primaryText={ val.name } />
              })
            }
          </List>
        </Paper>
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
