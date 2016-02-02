<gb-time>
  <gb-input icon="&#xE8AE;"
            label={opts.label}
            value={timestampToTimeStr(opts.value)}
            input-keyup={onKeyUp}
            input-onblur={onBlur}>
    <div class="hour-buttons">
      <gb-button btn-onclick={parent.decHour} size="small" circle outline>
        <i class="icon">&#xe314;</i>
      </gb-button>
      <gb-button btn-onclick={parent.incHour} size="small" circle outline>
        <i class='icon'>&#xe315;</i>
      </gb-button>
    </div>
  </gb-input>

  <style type="text/precss">
  .hour-buttons {
    display: table;
    margin-right: 4px;
    border-spacing: 4px;
  }

  .hour-buttons > * {
    display: table-cell;
  }
  </style>

  <script>
  const HOUR = 3600 * 1000;
  const timeRegex = new RegExp('([0-9]+):([0-9]+)');

  this.timestampToTimeStr = timestamp => {
    const date = new Date(timestamp);
    return padZeros(date.getHours(), 2) + ':' + padZeros(date.getMinutes(), 2);
  }

  this.timeStrToTimestamp = timeStr => {
    const match = timeRegex.exec(timeStr);
    if (match) {
      var date = new Date(this.opts.value);
      date.setHours(Math.min(parseInt(match[1]), 23));
      date.setMinutes(Math.min(parseInt(match[2]), 59));
      return date.getTime();
    } else {
      return undefined;
    }
  }

  this.onBlur = e => safeCall(this.opts.inputOnchange,
                              (typeof inputTimestamp !== 'undefined')
                                 ? this.opts.value
                                 : this.timeStrToTimestamp(e.target.value))

  this.decHour = () => safeCall(this.opts.inputOnchange, this.opts.value - HOUR)
  this.incHour = () => safeCall(this.opts.inputOnchange, this.opts.value + HOUR)
  </script>
</gb-time>
