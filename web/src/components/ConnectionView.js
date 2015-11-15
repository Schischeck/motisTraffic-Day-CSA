import React, { Component } from 'react';

export default class ConnectionView extends Component {
  constructor(props) {
    super(props);
  }

  render() {
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
    <div>
      <ul>
        { con.stops.filter((stop, i) => {
            return i === 1 || i === con.stops.length - 2 || stop.interchange;
          }).map(stop => {
            return ( <li>
                       { stop.name }
                     </li> );
          }) }
      </ul>
    </div>
    );
  }
}
