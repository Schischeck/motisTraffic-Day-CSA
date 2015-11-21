import React, { Component } from 'react';

import { Paper } from 'material-ui';

export default class PaddedPaper extends Component {
  constructor(props) {
    super(props);
  }

  render() {
    const style = this.props.style || {};
    style.padding = '20px';
    return (
    <Paper
           {...this.props}
           style={ style }>
      { this.props.children }
    </Paper>
    );
  }
}
