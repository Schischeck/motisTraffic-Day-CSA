!function() {
  var precss = {
    precss: function(colors, tag, css) {
      Object.keys(colors).forEach(function(key) {
        var re = new RegExp(key, 'g');
        css = css.replace(re, colors[key]);
      });

      var loops = 1000;
      var pos = 0;
      var lastBracketPos, openCount = 0, expression;
      while (--loops > 0) {
        pos = css.indexOf('precss.');
        if (pos == -1) {
          break;
        }

        openCount = 0;
        lastBracketPos = 0;
        for (var i = pos; i < css.length; i++) {
          if (css[i] === '(') {
            openCount++;
          } else if (css[i] === ')') {
            openCount--;
            if (openCount === 0) {
              lastBracketPos = i;
              break;
            }
          }
        }
        expression = css.substring(pos, lastBracketPos + 1);
        css = css.replace(expression, eval(expression));
      }

      return css;
    },
    each: function(collection, fun) {
      return collection.reduce((acc, el) => {
        return acc + '\n' + fun(el) + '\n';
      }, "");
    },
    components: function(c) {
      var i = parseInt(c.slice(1), 16);
      return {
        r: i >> 16,
        g: i >> 8 & 0x00FF,
        b: i & 0x0000FF
      };
    },
    toHex: function(c) {
      var i = 0;
      i += 0x1000000 + c.r * 0x10000;
      i += c.g * 0x100;
      i += c.b;
      return '#' + i.toString(16).slice(1);
    },
    blendColors: function blendColors(color1, color2, p) {
      var c1 = this.components(color1);
      var c2 = this.components(color2);
      return this.toHex({
        r: c1.r + Math.round((c2.r - c1.r) * p),
        g: c1.g + Math.round((c2.g - c1.g) * p),
        b: c1.b + Math.round((c2.b - c1.b) * p)
      });
    },
    DARKEN: function(c, p) {
      return this.blendColors(c, '#000000', parseInt(p) / 100);
    },
    LIGHTEN: function(c, p) {
      return this.blendColors(c, '#FFFFFF', parseInt(p) / 100);
    },
    DESATURATE: function(color) {
      var c = this.components(color);
      var avg = Math.round((c.r + c.g + c.b) / 3);
      return this.toHex({ r: avg, g: avg, b: avg });
    }
  }

  if (typeof exports === 'object') {
    // CommonJS
    module.exports = precss;
  } else if (typeof define === 'function' && typeof define.amd !== 'undefined') {
    // AMD
    define(function() {
      return (window.precss = precss);
    });
  } else {
    // browser
    window.precss = precss;
  }
}();