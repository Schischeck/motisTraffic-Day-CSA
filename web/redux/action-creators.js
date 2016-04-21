import type from 'redux/action-consts.js';
import server from 'util/server.js';

var actionCreators = {
  selectConnection: function(id) {
    return {
      type: type.SELECT_CONNECTION,
      payload: { id }
    };
  },

  swapLocations: function() {
    return { type: type.SWAP_LOCATIONS };
  },

  updateSearchForm: function(key, value) {
    return {
      type: type.SEARCH_FORM_UPDATE,
      payload: { key, value }
    };
  },

  searchAddTransportMode: function(where, transport) {
    return {
      type: type.SEARCH_ADD_TRANSPORT,
      payload: { where, transport }
    };
  },

  searchRemoveTransportMode: function(where, transport) {
    return {
      type: type.SEARCH_REMOVE_TRANSPORT,
      payload: { where, transport }
    };
  },

  receiveRouting: function(request, response) {
    return {
      type: type.RECEIVE_ROUTING,
      payload: {
        request: request,
        response: response.content.connections
      }
    }
  },

  requestRouting: function(dispatch, getState) {
    dispatch({ type: type.REQUEST_ROUTING });

    const s = getState().search.query;
    const date = new Date(s.date);
    const time = new Date(s.time);
    const queryDate = new Date(date.getFullYear(),
                               date.getMonth(),
                               date.getDate(),
                               time.getHours(),
                               time.getMinutes());
    const begin = Math.round(queryDate.getTime() / 1000);

    server.send({
      destination: {
        type: 'Module',
        target: '/routing'
      },
      content_type: 'RoutingRequest',
      content: {
        interval: { begin: begin, end: begin + 7200 },
        type: 'PreTrip',
        direction: 'Forward',
        path: [
          { name: s.from.name, eva_nr: s.from.eva || '' },
          { name: s.to.name, eva_nr: s.to.eva || '' }
        ],
        additional_edges: []
      }
    })
    .then(response => dispatch({
      type: type.RECEIVE_ROUTING,
      payload: {
        request: s,
        response: response.content.connections
      }
    }))
    .catch(err => dispatch({ type: type.RECEIVE_ROUTING_ERROR }));
  },

  resetQueryToLast: function() {
    return { type: type.RESET_QUERY_TO_LAST };
  }
}

export default actionCreators;
