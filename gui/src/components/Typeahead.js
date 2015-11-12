import React from 'react';

import { Container } from 'flux/utils';

import { TextField, List, ListItem, Paper } from 'material-ui/lib';

import style from './Typeahead.scss';

function mod(a, n) {
  return ((a % n) + n) % n;
}

export default class Typeahead extends React.Component {
  propTypes: {
    complete: React.PropTypes.func.isRequired
  }

  constructor(props) {
    super(props);
    this.state = {
      value: "",
      completions: [],
      selectedItemIndex: 0
    };
  }

  _fetchCompletions(evt) {
    this.props.complete(evt.target.value).then(response => {
      this.setState({
        completions: response
      });
    });
  }

  _onChange(e) {
    this.setState({
      value: e.target.value
    });
    this._fetchCompletions(e);
  }

  _selectGuess(value) {
    this.setState({
      value: value,
      completions: []
    });
  }

  _clearCompletions() {
    setTimeout(e => {
      this.setState({
        completions: []
      });
    }, 280);
  }

  _updateSelectedIndex(index) {
    this.setState({
      selectedItemIndex: mod(index, this.state.completions.length)
    });
  }

  _onKeyUp(e) {
    switch (e.keyCode) {
      // UP
      case 38:
        this._updateSelectedIndex(this.state.selectedItemIndex - 1);
        break;

      // DOWN
      case 40:
        this._updateSelectedIndex(this.state.selectedItemIndex + 1);
        break;

      // RETURN
      case 13:
      // ENTER
      case 14:
        this.setState({
          selectedItemIndex: 0,
          value: this.state.completions[this.state.selectedItemIndex].name,
          completions: []
        });
        break;

      // ESCAPE
      case 27:
        this._clearCompletions();
        break;
    }
  }

  render() {
    return (
    <div>
      <TextField
                 hintText={ this.props.name }
                 value={ this.state.value }
                 onChange={ this._onChange.bind(this) }
                 onKeyUp={ this._onKeyUp.bind(this) }
                 onBlur={ this._clearCompletions.bind(this) }>
      </TextField>
      <Paper
             ref={ 'suggestionbox' }
             zDepth={ 1 }
             className={ style.suggestions }>
        <List desktop={ true }>
          { this.state.completions.map((val, index) => {
              let style = {};
              if (this.state.selectedItemIndex === index) {
                style = {
                  backgroundColor: 'rgba(0, 0, 0, 0.0980392)'
                };
              }
              return <ListItem
                               style={ style }
                               hovered={ true }
                               key={ index }
                               primaryText={ val.name }
                               onMouseEnter={ this._updateSelectedIndex.bind(this, index) }
                               onClick={ this._selectGuess.bind(this, val.name) } />
            }) }
        </List>
      </Paper>
    </div>
    );
  }
}

export default Typeahead;
