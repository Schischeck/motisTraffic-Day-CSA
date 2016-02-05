<gb-button>
  <a name="btn" onclick={opts.btnOnclick} class={classesStr} style={opts.btnStyle}><yield/></a>

  <style type="text/precss" scoped>
  .gb-button {
    display: block;
    box-sizing: content-box;
    border: 1px solid transparent;
    border-radius: 3px;
    box-shadow: 0 2px 4px rgba(0,0,0,.16);
    text-align: center;
    font-size: 16px;
    white-space: nowrap;
    transition: all .3s;
    position: relative;
    padding: 0 10px;
  }

  .gb-button:after {
    content: "";
    position: absolute;
    top: 0;
    left: 0;
    width: 100%;
    height: 100%;
    border-radius: 3px;
    box-shadow: inset 0 2px 2px rgba(0,0,0,.16);
    opacity: 0;
    transition: all .3s;
  }

  .gb-button.gb-button-circle:after {
    border-radius: 9999px;
  }

  .gb-button:active:after {
    opacity: 1;
  }

  .gb-button, .gb-button > * { cursor: pointer; }

  .gb-button-small {
    height: 28px;
    line-height: 28px;
    font-weight: 400;
  }

  .gb-button-medium {
    height: 48px;
    line-height: 48px;
    font-weight: 500;
  }

  .gb-button-large {
    height: 64px;
    line-height: 64px;
    font-weight: 700;
  }

  .gb-button-circle {
    border-radius: 9999px;
    padding: 0;
  }

  .gb-button-circle.gb-button-small { width: 28px; }
  .gb-button-circle.gb-button-medium { width: 48px; }
  .gb-button-circle.gb-button-large { width: 64px; }

  .gb-button-circle.gb-button-small > * {
    font-size: 28px;
   }
  .gb-button-circle.gb-button-medium > * {
    font-size: 48px;
  }
  .gb-button-circle.gb-button-large > * {
    font-size: 64px;
  }

  .gb-button-small > * {
    line-height: 28px;
   }
  .gb-button-medium > * {
    line-height: 48px;
  }
  .gb-button-large > * {
    line-height: 64px;
  }

  .gb-button-outline.gb-button-small { border-width: 1px; }
  .gb-button-outline.gb-button-medium { border-width: 2px; }
  .gb-button-outline.gb-button-large { border-width: 3px; }

  .gb-button-outline {
    background-color: white;
  }

  precss.each(Object.keys(colors).map(key => {
                return { key, value: colors[key] };
              }), color => {
    return `
      .gb-button-${color.key} {
        background-color: ${color.value};
        color: white;
      }

      .gb-button-${color.key}:hover {
        background: ${precss.LIGHTEN(color.value, 8)};
      }

      .gb-button-${color.key}:active {
        background: ${precss.DARKEN(color.value, 5)};
      }

      .gb-button-${color.key}.gb-button-outline {
        color: ${color.value};
        border-color: ${color.value};
        background-color: white;
      }`;
  })
  </style>

  <script>
    function makeClass(outline, circle, size, color, userClass) {
      var clasz = ['gb-button'];
      if (size) {
        clasz.push('gb-button-' + size);
      } else {
        clasz.push('gb-button-medium');
      }
      if (circle) {
        clasz.push('gb-button-circle');
      }
      if (outline) {
        clasz.push('gb-button-outline');
      }
      clasz.push('gb-button-' + (color || 'PRIMARY_COLOR'));
      clasz.push('disable-select');
      clasz.concat(userClass);
      return clasz.join(' ');
    }
    var size = this.opts.size || 'medium';
    var circle = typeof this.opts.circle !== 'undefined';
    var outline = typeof this.opts.outline !== 'undefined';
    this.classesStr = makeClass(outline, circle, this.opts.size, this.opts.color, this.opts.classes);
  </script>
</gb-button>
