!function() {
  const EXPORT_TOKEN = 'export '
  const EXPORT_DEFAULT_TOKEN = EXPORT_TOKEN + 'default ';
  const IMPORT_TOKEN = 'import ';
  const IMPORT_FROM_TOKEN = ' from ';

  function parseModuleInfo(js) {
    var exports = "{\n";
    var importDefs = "";
    var imports = "";

    js = js.split('\n').filter(line => {
      line = line.trim().slice(0, -1);
      if (line.startsWith(EXPORT_DEFAULT_TOKEN)) {
        var variableName = line.substr(EXPORT_DEFAULT_TOKEN.length);
        exports += "      default: " + variableName + ",\n";
        return false;
      } else if (line.startsWith(IMPORT_TOKEN)) {
        var fromPos = line.indexOf(IMPORT_FROM_TOKEN);
        importDefs += ', ' + line.substring(IMPORT_TOKEN.length, fromPos);

        var moduleKey = line.substr(IMPORT_TOKEN.length + fromPos - 1);
        imports += ", window.JAVASCRIPT_MODULES[" + moduleKey + "].default"

        return false;
      }
      return true;
    }).join('\n');

    exports += "  }";

    return {
      cleanedJs: js,
      exports: exports,
      imports: imports.substr(2),
      importDefs: importDefs.substr(2)
    };
  }

  function compileJsModule(js, opts, tag)  {
    var moduleInfo = parseModuleInfo(js);
    return 'window.JAVASCRIPT_MODULES["' + tag + '"] = ' +
           'function(' + moduleInfo.importDefs + ') {\n' +
             moduleInfo.cleanedJs +
             '\n' +
             '  return ' + moduleInfo.exports + ';\n' +
           '}.bind(this)(' + moduleInfo.imports + ');';
  }

  if (typeof exports === 'object') {
    // CommonJS
    module.exports = compileJsModule;
  } else if (typeof define === 'function' && typeof define.amd !== 'undefined') {
    // AMD
    define(function() {
      return (window.compileJsModule = compileJsModule)
    });
  } else {
    // browser
    window.compileJsModule = compileJsModule;
  }
}();