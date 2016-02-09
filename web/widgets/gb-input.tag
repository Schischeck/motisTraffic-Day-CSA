<gb-input>
  <div class="label" if={opts.label}>{opts.label}</div>
  <div class={'gb-input-group': true, 'gb-input-group-selected': isFocussed}>
    <div if={opts.icon} class="gb-input-icon">
      <i class="icon" onclick={focus}>{opts.icon}</i>
    </div>
    <input name="input"
           type="text"
           class="gb-input"
           value={opts.value}
           placeholder={opts.placeholder}
           onfocus={onFocus}
           onblur={onBlur}
           onclick={opts.inputOnclick}
           onkeyup={onKeyUp} />
     <div class="gb-input-widget">
       <yield/>
     </div>
  </div>

  <style scoped type="text/precss">
  .gb-input-group-selected {
    border-color: #5f93e7 !important;
    outline: 0;
  }

  .gb-input {
    display: table-cell;
    width: 100%;
    font-size: 15px;
    padding: 16px 0 16px 16px;
    border: none;
    line-height: 1;
    background: white;
    color: #3c374b;
  }

  .gb-input-group {
    display: table;
    vertical-align: middle;
    width: 100%;
    border-style: solid;
    border-color: #dbe0ed;
    border-width: 0 0 1px 0;
    margin-bottom: 16px;
    height: 50px;
    vertical-align: middle;
    position: relative;
    background-color: #fff;
    padding: 0;
    transition: .2s border-color, .2s box-shadow;
  }

  .gb-input-icon {
    display: table-cell;
    vertical-align: middle;
    width: 1%;
    color: #abb7cc;
  }

  .gb-input-icon .icon {
    padding-right: 0;
    font-size: 30px;
    margin-left: 10px;
    margin-top: 4px;
    cursor: text;
  }

  .gb-input-widget {
    display: table-cell;
    vertical-align: middle;
    margin-right: 10px;
    width: 1%;
  }
  </style>

  <script>
  this.focus = () => {
    this.input.focus();
    this.input.setSelectionRange(0, 0);
  }
  this.onFocus = () => this.isFocussed = true
  this.onBlur = e => {
    this.isFocussed = false;
    if (this.opts.inputOnblur) {
      this.opts.inputOnblur(e);
    }
  }
  this.onKeyUp = e => safeCall(this.opts.inputKeyup, e)
  </script>
</gb-input>
