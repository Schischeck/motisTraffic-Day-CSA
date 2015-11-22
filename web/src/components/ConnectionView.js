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

  makeTrainNames(con, from, to, key) {
    return (<ul key={ key } className={style.trainNames}>
              { con.transports.filter(t => {
                  return t.move.range.from >= from && t.move.range.to <= to;
                }).map(t => {
                  return <li>
                           { t.move.name }
                         </li>;
                }) }
            </ul>);
  }

  render() {
    if (this.props.waiting) {
      return (<CircularProgress
                                mode="indeterminate"
                                style={ {  'margin': '10px auto',  'display': 'block'} } />);
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
        { con.stops.map((stop, i) => {
            return {
              type: 'stop',
              stop,
              i
            };
          }).filter((el, i) => {
            return i === 1 || i === con.stops.length - 2 || el.stop.interchange;
          }).reduce((acc, curr) => {
            if (acc.length !== 0) {
              acc.push({
                type: 'transport',
                from: acc[acc.length - 1].i,
                to: curr.i
              });
            }
            acc.push(curr);
            return acc;
          }, []).map((el, index) => {
            if (el.type == 'stop') {
              const arr = el.i !== 1 ? this.makeArrivalElement(el.stop.arrival.time) : <div />;
              const dep = el.i !== con.stops.length - 2 ? this.makeArrivalElement(el.stop.departure.time) : <div />;
              return ( <li
                           key={ index }
                           className={ style.stop }>
                         { arr }
                         <div className={ style.stationName }>
                           { el.stop.name }
                         </div>
                         { dep }
                       </li> );
            } else {
              return this.makeTrainNames(con, el.from, el.to, index);
            }
          }) }
      </ul>
    </div>
    );
  }
}
