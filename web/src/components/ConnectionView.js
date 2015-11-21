import React, { Component } from 'react';

import { CircularProgress } from 'material-ui';

import style from './ConnectionView.scss';

export default class ConnectionView extends Component {
  constructor(props) {
    super(props);
  }

  toTimeString(timestamp) {
    const date = new Date(timestamp * 1000);
    return date.toLocaleTimeString(navigator.language, {
      hour: '2-digit',
      minute: '2-digit'
    });
  }

  makeArrivalElement(time) {
    return (<div className={ style.arrival }>
              { this.toTimeString(time) }
            </div>);
  }

  makeDepartureElement(time) {
    return (<div className={ style.departure }>
              { this.toTimeString(time) }
            </div>);
  }

  makeAttributes(con, interchange) {
    return <span className={ style.trainId }>{ con.transports[interchange].move.name }</span>;
  }

  render() {
    if (this.props.waiting) {
      return (<CircularProgress
                                mode="indeterminate"
                                style={ { 'margin': '10px auto', 'display': 'block'} } />);
    }

    if (this.props.showError) {
      return (<div>
                <span>An error occured</span>
              </div>);
    }

    if (this.props.connections.length === 0) {
      return (<div>
                <span>No connections found</span>
              </div>);
    }

    const con = this.props.connections[this.props.connections.length - 1];
    return (
    <div className={ style.conWrap }>
      <ul className={ style.con }>
        { con.stops.filter((stop, i) => {
            return i === 1 || i === con.stops.length - 2 || stop.interchange;
          }).map((stop, i, src) => {
            console.log(stop);
            const attrs = this.makeAttributes(con, i + 1);
            const arr = i !== 0 ? this.makeArrivalElement(stop.arrival.time) : <div />;
            const dep = i !== src.length - 1 ? this.makeArrivalElement(stop.arrival.time) : <div />;
            return ( <li
                         key={ i }
                         className={ style.stop }>
                       { arr }
                       <div className={ style.stationname }>
                         { stop.name }
                         { attrs }
                       </div>
                       { dep }
                     </li> );
          }) }
      </ul>
    </div>
    );
  }
}
