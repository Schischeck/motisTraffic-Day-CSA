import React, { Component } from 'react';

import { CircularProgress, FloatingActionButton } from 'material-ui';

import PaddedPaper from './PaddedPaper';
import RoutingForm from './RoutingForm';
import Map from './Map';
import ServerConnectionIndicator from './ServerConnectionIndicator';
import Timeline from './timeline/MotisTimeline';
import DetailView from './timeline/MotisDetailView';

import Server from '../Server';
import StationGuesserRequest from '../Messages/StationGuesserRequest';

import style from './App.scss';
import iconcss from './MaterialIcons.scss';
const materialicons = iconcss['material-icons'];
export class App extends Component {
  constructor(props) {
    super(props);
    const cached = JSON.parse(localStorage.getItem('motiscache')) || {};
    const lastReq = cached.req;
    const lastFrom = lastReq ? lastReq.content.path[0].name : '';
    const lastTo = lastReq ? lastReq.content.path[1].name : '';
    const lastDate = lastReq ? new Date(lastReq.content.interval.begin * 1000) : new Date();
    this.state = {
      'connections': cached.connections || [],
      'lastFrom': lastFrom,
      'lastTo': lastTo,
      'lastDate': lastDate,
      'showError': false,
      'waiting': false,
      'showResults': !!cached.connections,
      'visibleSearch': true
    };
  }

  getRouting() {
    this.setState({
      'waiting': true,
    });

    const req = this.refs.routingform.getRequest();
    Server.sendMessage(req).then(response => {
      this.setState({
        'connections': response.content.connections,
        'showError': false,
        'waiting': false,
        'showResults': true
      });
      localStorage.setItem('motiscache', JSON.stringify({
        'connections': response.content.connections,
        'req': req
      }));
    });
  }

  hideSearch() {
    this.setState({
      'visibleSearch': false
    });
  }

  showSearch() {
    this.setState({
      'visibleSearch': true
    });
  }

  guessStation(input) {
    return new Promise(resolve => {
      Server.sendMessage(new StationGuesserRequest(input, 7)).then(response => {
        resolve(response.content.guesses);
      });
    });
  }

  render() {
    const results = this.state.waiting
                    ? <CircularProgress
                          mode="indeterminate"
                          style={ { 'margin': '50px auto', 'display': 'block'} } />
                    : <DetailView connection={ this.state.connections[0] } />;
    const layer = this.state.visibleSearch ? <div className={ style.layer }/> : <div />;
    const search = this.state.visibleSearch ? <PaddedPaper
        className={ style.overlay }
        zDepth={ 1 }>
          <div className={ style.topbar }>
            <ServerConnectionIndicator iconStyle={{ 'color': '#999' }}/>
          </div>
          <RoutingForm
                       ref="routingform"
                       initFrom={ this.state.lastFrom }
                       initTo={ this.state.lastTo }
                       initDate={ this.state.lastDate }
                       disabled={ this.state.waiting }
                       onRequestRouting={ this.getRouting.bind(this) } />
          { results }
      </PaddedPaper> : <div />;
    const searchButton = (<FloatingActionButton
                            onClick={ this.showSearch.bind(this) }
                            secondary={ true }
                            style = { { 'margin': '2px' } }
                            >
        <i className={ materialicons }>&#xe8b6;</i>
                            </FloatingActionButton>);
    const railVizButton = (<FloatingActionButton
                            onClick={ this.hideSearch.bind(this) }
                            secondary={ true }
                            style = { { 'margin': '2px' } }
                            >
        <i className={ materialicons }>&#xe8f4;</i>
                            </FloatingActionButton>);

    const statusButton = (<FloatingActionButton
                            onClick={ this.hideSearch.bind(this) }
                            secondary={ true }
                            style = { { 'margin': '2px' } }
                            >
        <i className={ materialicons }>&#xe8b8;</i>
                            </FloatingActionButton>);
    return (
    <div className={ style.app }>
      <Map />

      { layer }
      <div className={ style.control }>{ searchButton }{ railVizButton }{ statusButton }</div>
      { search }
    </div>
    );
  }
}
