import {Component} from 'react';
import Actions from '../flux-infra/Actions';

export default class BaseComponent extends Component {
  constructor(props) {
    super(props);
  }

  static componentIdCounter = 0;
  componentId = ++BaseComponent.componentIdCounter;
}
