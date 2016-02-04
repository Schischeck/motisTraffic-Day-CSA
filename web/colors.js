!function() {
  var colors = {
    'PRIMARY_COLOR': '#4971E7',
    'TEXT_COLOR': '#3C374B',
    'FORM_ITEMS_COLOR': '#ABB7CC',
  };

  if (typeof exports === 'object') {
    // CommonJS
    module.exports = colors;
  } else if (typeof define === 'function' && typeof define.amd !== 'undefined') {
    // AMD
    define(function() {
      return (window.colors = colors)
    });
  } else {
    // browser
    window.colors = colors;
  }
}();