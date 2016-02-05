'use strict';

const fs = require('fs');

const cc = require('closure-compiler');

var riot = require('riot-compiler');
const compileJsModule = require('../util/module-compiler.js');
const precss = require('../util/precss.js');
const colors = require('../colors.js');
riot.parsers.css.precss = precss.precss.bind(null, colors);
riot.parsers.js.module = compileJsModule;

const CleanCSS = require('clean-css');

function readAttribute(line, name) {
  const startToken = name + '="';

  const start = line.indexOf(startToken);
  if (start === -1) {
    return null;
  }

  const end = line.indexOf('"', start + startToken.length)
  if (end === -1) {
    return null;
  }

  return line.substring(start + startToken.length, end);
}

const lineReader = require('readline').createInterface({
  input: fs.createReadStream('index.html')
});

var scriptFiles = [];
function parseScriptTag(line) {
  const type = readAttribute(line, 'type') || 'js';
  const prodSrc = readAttribute(line, 'prod-src');
  const src = readAttribute(line, 'src');
  const file = (prodSrc !== null) ? prodSrc : src;

  if (file && file.length !== 0) {
    scriptFiles.push({file, type});
  }
}

var styleFiles = [];
function parseStyleInclude(line) {
  const src = readAttribute(line, 'href')
  if (src) {
    styleFiles.push(src);
  }
}

lineReader.on('line', function(line) {
  if (line.indexOf('<script') !== -1) {
    parseScriptTag(line)
  } else if (line.indexOf('rel="stylesheet"') !== -1) {
    parseStyleInclude(line);
  }
});

lineReader.on('close', function() {
  let bundle = '';
  scriptFiles.every(src => {
    let content = fs.readFileSync(src.file, 'utf-8');
    let processed = false;
    if (src.type === 'module') {
      processed = compileJsModule(content, {}, src.file);
    } else if (src.type === 'riot/tag') {
      processed = riot.compile(content, true);
    } else if (src.type === 'js') {
      processed = content;
    }
    if (processed) {
      console.log(src.file, '(' + Math.ceil(processed.length / 1000) + 'kb)');
      bundle = bundle.concat(processed + '\n');
    }
    // continue loop
    return true;
  });

  var style = new CleanCSS().minify(styleFiles).styles;
  console.log('style.css', '(' + Math.ceil(style.length / 1000) + 'kb)')

  cc.compile(bundle, {
    'language_in': 'ECMASCRIPT6',
    'language_out': 'ECMASCRIPT5'
  }, function(err, script) {
    var indexHtml = '<!doctype html>' +
                    '<html>' +
                      '<head>' +
                        '<title>MOTIS</title>' +
                        '<style>' + style + '</style>' +
                        '<meta content="text/html;charset=utf-8" http-equiv="Content-Type">' +
                        '<meta content="utf-8" http-equiv="encoding">' +
                      '</head>' +
                      '<body>' +
                        '<script>' + script + '</script>' +
                      '</body>' +
                    '</html>';
    fs.mkdir('./dist', () => fs.writeFileSync('./dist/index.html', indexHtml));
  });
});


