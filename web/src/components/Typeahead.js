import React from 'react';

import { TextField, List, ListItem, Paper } from 'material-ui/lib';

import style from './Typeahead.scss';

function mod(a, n) {
  return ((a % n) + n) % n;
}

export default class Typeahead extends React.Component {
  propTypes: {
    complete: React.PropTypes.func.isRequired,
    name: React.PropTypes.string
  }

  constructor(props) {
    super(props);
    this.state = {
      value: '',
      completions: [],
      selectedItemIndex: 0,
      selectedCompletion: undefined
    };
  }

  getValue() {
    if (this.state.selectedCompletion !== undefined) {
      return this.state.selectedCompletion;
    }
    return {
      name: this.state.value
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

  _selectGuess(index) {
    this.setState({
      selectedItemIndex: 0,
      selectedCompletion: this.state.completions[index],
      value: this.state.completions[index].name,
      completions: []
    });
  }

  _clearCompletions() {
    setTimeout(() => {
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
        this._selectGuess(this.state.selectedItemIndex);
        break;

      // ESCAPE
      case 27:
        this._clearCompletions();
        break;

      default: // nop
    }
  }

  render() {
    return (
    <div>
      <TextField
                 floatingLabelText={ this.props.hintText }
                 value={ this.state.value }
                 onChange={ this._onChange.bind(this) }
                 onKeyUp={ this._onKeyUp.bind(this) }
                 onBlur={ this._clearCompletions.bind(this) } />
      <Paper
             zDepth={ 1 }
             className={ style.suggestions }>
        <List desktop={ true }>
          { this.state.completions.map((val, index) => {
              let selectedStyle = {};
              if (this.state.selectedItemIndex === index) {
                selectedStyle = {
                  backgroundColor: 'rgba(0, 0, 0, 0.0980392)'
                };
              }
              return ( <ListItem
                                 style={ selectedStyle }
                                 hovered={ true }
                                 key={ index }
                                 primaryText={ val.name }
                                 onMouseEnter={ this._updateSelectedIndex.bind(this, index) }
                                 onClick={ this._selectGuess.bind(this, index) } /> );
            }) }
        </List>
      </Paper>
    </div>
    );
  }
}
