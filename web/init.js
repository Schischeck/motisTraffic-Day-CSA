import reducers from 'redux/reducers.js';
import actions from 'redux/action-creators.js';

const appReducers = Redux.combineReducers(reducers);

const statePersisterMiddleware = store => next => action => {
  const result = next(action);
  window.localStorage.setItem('motis-search', JSON.stringify(store.getState().search));
  return result;
};

const thunkMiddleware = store => next => action => {
  if (typeof action === 'function') {
    action(store.dispatch, store.getState);
  } else {
    next(action);
  }
}

const logger = store => next => action => {
  console.group(action.type)
  console.info('current state', store.getState())
  console.info('dispatching', action)
  const result = next(action)
  console.log('next state', store.getState())
  console.groupEnd(action.type)
  return result
}

const createStoreWithMiddleware = Redux.applyMiddleware(
  statePersisterMiddleware,
  thunkMiddleware
)(Redux.createStore);

window.init = () => riot.mount('app', {
  store: createStoreWithMiddleware(appReducers)
})