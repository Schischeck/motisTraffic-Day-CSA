import c from 'search/timeline/constants.js';

const MINUTE = 60 * 1000;

function getBeginAndEnd(cons) {
  var min = cons[0][0].begin.getTime();
  var max = cons[0][cons[0].length - 1].end.getTime();
  cons.forEach(sections => {
    sections.forEach(section => {
      min = Math.min(section.begin.getTime(), min)
      max = Math.max(section.end.getTime(), max)
    })
  })
  return { begin: new Date(min), end: new Date(max) };
}

function getScale(begin, end) {
  const scales = [
    { start: 0, end: 5, scale: 1 * MINUTE },
    { start: 5, end: 10, scale: 2 * MINUTE },
    { start: 10, end: 30, scale: 5 * MINUTE },
    { start: 30, end: 90, scale: 10 * MINUTE },
    { start: 90, end: 180, scale: 30 * MINUTE },
    { start: 3 * 60, end: 6 * 60, scale: 60 * MINUTE },
    { start: 6 * 60, end: 12 * 60, scale: 120 * MINUTE },
    { start: 12 * 60, end: 48 * 60, scale: 240 * MINUTE },
  ];

  const minutes = (end.getTime() - begin.getTime()) / (60 * 1000);
  var scale;
  scales.forEach(s => {
    if (minutes >= s.start && minutes < s.end) {
      scale = s.scale;
    }
  })

  return scale || 480 * MINUTE;
}

function floorToScale(t, scale) {
  t.setSeconds(0, 0);

  const midnight = new Date(t.getTime());
  midnight.setHours(0, 0, 0, 0);

  const sinceMidnight = t.getTime() - midnight.getTime();
  return new Date(t.getTime() - (sinceMidnight % scale));
}

function ceilToScale(t, scale) {
  t.setSeconds(0, 0);

  const midnight = new Date(t.getTime());
  midnight.setHours(0, 0, 0, 0);

  const sinceMidnight = t.getTime() - midnight.getTime();
  return new Date(t.getTime() + (scale - (sinceMidnight % scale)));
}

const calculateTimelineSettings = function(cons, width) {
  const beginAndEnd = getBeginAndEnd(cons);
  const scale = getScale(beginAndEnd.begin, beginAndEnd.end);
  const begin = floorToScale(beginAndEnd.begin, scale);
  const end = ceilToScale(beginAndEnd.end, scale);
  return {
    begin: begin,
    end: end,
    scale: scale,
    start: floorToScale(new Date(begin.getTime()), scale),
    totalCuts: Math.ceil((end.getTime() - begin.getTime()) / scale) + 1,
    timeToXIntercept: function(t) {
      const period = end.getTime() - begin.getTime();
      const untilT = t.getTime() - begin.getTime();
      const percent = untilT / period;
      return c.PADDING + (width - c.PADDING * 2) * percent;
    }
  };
};

export default calculateTimelineSettings;