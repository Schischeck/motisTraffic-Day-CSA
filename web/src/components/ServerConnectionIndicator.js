import React, { Component } from 'react';
import { Container } from 'flux/utils';

import { IconButton } from 'material-ui';

import ConnectionStateStore from '../ConnectionStateStore';

import iconcss from './MaterialIcons.scss';
const materialicons = iconcss['material-icons'];

class ServerConnectionIndicator extends Component {
  static getStores() {
    return [
      ConnectionStateStore
    ];
  }

  static calculateState(prevState) {
    return {
      connected: ConnectionStateStore.isConnected(),
    };
  }

  render() {
    if (this.state.connected) {
      return <IconButton
                         {...this.props}
                         iconClassName={ materialicons }>
               cloud_done
             </IconButton>;
    } else {
      return <IconButton
                         {...this.props}
                         iconClassName={ materialicons }>
               cloud_off
             </IconButton>;
    }
  }
}

export default Container.create(ServerConnectionIndicator);
