import React, { Component } from 'react';

import { Paper } from 'material-ui';

export default class PaddedPaper extends Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (
    <Paper
           {...this.props}
           style={ { 'padding': '20px'} }>
      { this.props.children }
    </Paper>
    );
  }
}
