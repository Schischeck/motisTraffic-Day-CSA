import type from 'redux/action-consts.js';

function compareTags(a, b) {
  return a.id < b.id ? -1 : (a.id === b.id ? 0 : 1)
}

function storedState() {
  var state = JSON.parse(window.localStorage.getItem('motis-search'));
  if (state) {
    state.result.loading = false;
    state.result.error = false;
  }
  return state;
}

function search(state, action) {
  state = state ||
          storedState() ||
          {
            query: {
              from: { name: '' },
              to: { name: '' },
              date: new Date().getTime(),
              time: new Date().getTime(),
              transports: { from: [], to: [] },
              dir: 'Forward'
            },
            result: {
              error: false,
              request: undefined,
              response: undefined,
              selectedConnection: undefined,
            }
          };

  if (action.type !== type.RECEIVE_ROUTING_ERROR) {
    state.result.error = false;
  }

  var tmp;
  switch (action.type) {
    case type.SWAP_LOCATIONS:
      tmp = state.query.from
      state.query.from = state.query.to
      state.query.to = tmp
      break

    case type.SEARCH_FORM_UPDATE:
      state.query[action.payload.key] = action.payload.value
      break

    case type.SEARCH_ADD_TRANSPORT:
      state.query.transports[action.payload.where].push(action.payload.transport)
      state.query.transports[action.payload.where].sort(compareTags)
      break

    case type.SEARCH_REMOVE_TRANSPORT: {
      const transports = state.query.transports[action.payload.where]
      const index = transports.indexOf(action.payload.transport)
      if (index > -1) {
        transports.splice(index, 1)
      }
      break
    }

    case type.REQUEST_ROUTING:
      state.result.loading = true
      break

    case type.RECEIVE_ROUTING:
      state.result = {
        error: false,
        loading: false,
        selectedConnection: undefined,
        response: action.payload.response,
        request: JSON.parse(JSON.stringify(action.payload.request))
      }
      break

    case type.RECEIVE_ROUTING_ERROR:
      state.result = {
        error: true,
        loading: false,
        selectedConnection: undefined,
        response: undefined,
        request: undefined
      }
      break

    case type.RESET_QUERY_TO_LAST:
      state.query = JSON.parse(JSON.stringify(state.result.request))
      break

    case type.SELECT_CONNECTION:
      state.result.selectedConnection = action.payload.id;
      break
  }

  return state;
}

export default { search };
