import React from 'react';
import ReactDOM from 'react-dom';
import { App } from './components/App';
import injectTapEventPlugin from 'react-tap-event-plugin';

import '../base.scss';

//Needed for onTouchTap
//Can go away when react 1.0 release
//Check this repo:
//https://github.com/zilverline/react-tap-event-plugin
injectTapEventPlugin();

ReactDOM.render(<App />, document.getElementById('root'));
