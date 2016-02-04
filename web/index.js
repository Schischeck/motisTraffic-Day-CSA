var app = document.createElement('app');
document.getElementsByTagName('body')[0].appendChild(app);

if (riot.parsers) {
  riot.parsers.css.precss = precss.precss.bind(null, colors);
  riot.parsers.js.module = compileJsModule;
}
loadJsModules(() => { init(); });
