<gb-date>
  <gb-input icon="&#xE878;"
            label={opts.label}
            value={timestampToDateStr(opts.value)}
            input-onclick={showAndUpdate}
            input-keyup={onKeyUp}>
    <div class="day-buttons">
      <gb-button btn-onclick={parent.decreaseDate} size="small" circle outline>
        <i class="icon">&#xe314;</i>
      </gb-button>
      <gb-button btn-onclick={parent.increaseDate} size="small" circle outline>
        <i class='icon'>&#xe315;</i>
      </gb-button>
    </div>
  </gb-input>
  <div if={isVisible} class="paper calendar" onclick={preventDismiss}>
    <div class="month">
      <i class="icon" onclick={decreaseMonth}>&#xe314;</i>
      <span class="month-name">
        { date.toLocaleDateString(this.currentLocale, {month: 'long'}) }
        { date.getFullYear() }
      </span>
      <i class='icon' onclick={increaseMonth}>&#xe315;</i>
    </div>
    <ul class="weekdays">
      <li each={ day in calendarDays.slice(0, 7) }>
        { day.date.toLocaleDateString(this.currentLocale, {weekday: 'short'}) }
      </li>
    </ul>
    <ul class="calendardays">
      <li each={ day in calendarDays }
          onclick={ selectDate }
          class={
            'out-of-month': !day.inMonth,
            'in-month': day.inMonth,
            'today': day.today,
            'selected': day.selected
          }>
        { day.date.getDate() }
      </li>
    </ul>
  </div>

  <style scoped type="text/precss">
  @keyframes fadedown {
      0% {
        opacity: 0;
        margin-top: -50px;
      }
      100% {
        opacity: 1;
        margin-top: -10px;
      }
  }

  .calendar {
    animation-name: fadedown;
    animation-duration: .1s;
  }

  .day-buttons {
    display: table;
    margin-right: 4px;
    border-spacing: 4px;
  }

  .day-buttons > * {
    display: table-cell;
  }

  .month {
    display: flex;
    justify-content: space-between;
    width: 100%;
    text-align: center;
    padding-top: 10px;
    font-size: .85em;
    font-weight: 700;
    text-transform: uppercase;
    color: #ee3897;
  }

  .month-name {
    flex: 1;
  }

  .month .icon {
    font-size: 1.5em;
    margin: 0 10px;
    cursor: pointer;
  }

  .weekdays, .calendardays {
    margin: 0;
    padding: 0;
    list-style: none;
  }

  .weekdays li, .calendardays li {
    display: table-cell;
    width: 40px;
    height: 40px;
    line-height: 40px;
    float: left;
    text-align: center;
    border-radius: 3px;
  }

  .calendardays li {
    font-size: 13px;
    cursor: pointer;
  }

  .calendardays li:hover {
    background-color: precss.LIGHTEN("PRIMARY_COLOR", 80);
  }

  .weekdays li {
    color: #96a2ba;
    font-size: .75em;
    font-weight: 700;
    text-transform: uppercase;
  }

  .in-month {
    font-weight: bold !important;
  }

  .out-of-month {
    font-weight: lighter;
  }

  .selected, .selected:hover {
    color: white;
    background-color: precss.LIGHTEN("PRIMARY_COLOR", 30) !important;
  }

  .today {
    border: 1px solid PRIMARY_COLOR;
  }
  </style>

  <script>
  this.mixin('i18n');
  this.mixin('dismiss');

  const kDateFormat = {
    en: {
      regex: '([0-9]+)/([0-9]+)/([0-9]{4})',
      seperator: '/',
      order: { YEAR: 3, MONTH: 1, DAY: 2 }
    },
    de: {
      regex: '([0-9]+)\\.([0-9]+)\\.([0-9]{4})',
      seperator: '.',
      order: { YEAR: 3, MONTH: 2, DAY: 1 }
    }
  };

  this.date = new Date()

  this.dateStrToTimestamp = dateStr => {
    const format = kDateFormat[this.currentLocale];
    const match = new RegExp(format.regex).exec(dateStr);
    if (!match) {
      return null;
    }

    var date = new Date();
    date.setHours(0, 0, 0, 0);
    date.setYear(parseInt(match[format.order.YEAR]));
    date.setMonth(parseInt(match[format.order.MONTH]) - 1);
    date.setDate(parseInt(match[format.order.DAY]));
    return date;
  }

  this.timestampToDateStr = timestamp => {
    const date = new Date(timestamp);
    const format = kDateFormat[this.currentLocale];

    var dateStrArr = ['', '', ''];
    dateStrArr[format.order.YEAR - 1] = date.getYear() + 1900;
    dateStrArr[format.order.MONTH - 1] = date.getMonth() + 1;
    dateStrArr[format.order.DAY - 1] = date.getDate();
    return dateStrArr.join(format.seperator);
  }

  this.onKeyUp = e => {
    var date = this.dateStrToTimestamp(e.target.value);
    if (date) {
      safeCall(this.opts.inputOnchange, date.getTime());
    }
  }

  this.increaseMonth = () => {
    var date = new Date(this.opts.value);
    date.setMonth(date.getMonth() + 1);
    safeCall(this.opts.inputOnchange, date.getTime());
  }

  this.decreaseMonth = () => {
    var date = new Date(this.opts.value);
    date.setMonth(date.getMonth() - 1);
    safeCall(this.opts.inputOnchange, date.getTime());
  }

  this.increaseDate = () => {
    this.preventDismiss();
    var date = new Date(this.opts.value);
    date.setDate(date.getDate() + 1);
    safeCall(this.opts.inputOnchange, date.getTime());
  }

  this.decreaseDate = () => {
    this.preventDismiss();
    var date = new Date(this.opts.value);
    date.setDate(date.getDate() - 1);
    safeCall(this.opts.inputOnchange, date.getTime());
  }

  this.selectDate = e => {
    this.hide();
    safeCall(this.opts.inputOnchange, e.item.day.date.getTime());
  };

  this.buildCalendar = () => {
    if (this.opts.value == this.date.getTime()) {
      return;
    }

    this.date = new Date(this.opts.value);

    this.today = new Date();
    this.today.setHours(0, 0, 0, 0);

    this.currentDateString = this.date.toLocaleDateString(this.currentLocale);

    var monthBegin = new Date(this.date.getTime());
    monthBegin.setDate(1);

    var cal = new Date(monthBegin.getTime());
    cal.setHours(cal.getHours() - modulo(monthBegin.getDay() - 1, 7) * 24);

    var monthEnd = new Date(monthBegin.getTime());
    monthEnd.setMonth(monthEnd.getMonth() + 1);

    var calEnd = new Date(monthEnd.getTime());
    calEnd.setHours(calEnd.getHours() + (7 - modulo(calEnd.getDay() - 1, 7)) * 24);

    this.calendarDays = [];
    var currentDay = new Date(cal.getTime());
    while (currentDay < calEnd) {
      this.calendarDays.push({
        today: this.today.getTime() == currentDay.getTime(),
        selected: this.date.getTime() == currentDay.getTime(),
        inMonth: currentDay >= monthBegin && currentDay < monthEnd,
        date: new Date(currentDay.getTime())
      });
      currentDay.setDate(currentDay.getDate() + 1);
    }
  }

  this.on('update', () => this.buildCalendar());
  </script>
</gb-date>
