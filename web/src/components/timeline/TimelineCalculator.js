var MINUTE = 60*1000;

function getBeginAndEnd(cons) {
  let begin = cons[0][0].begin;
  let end = cons[0][cons[0].length - 1].end;
  for (let i = 0; i < cons.length; i++) {
    let c = cons[i];
    for (let j = 0; j < c.length; j++) {
      if (c[j].begin < begin) {
        begin = c[j].begin;
      }
      if (c[j].end > end) {
        end = c[j].end;
      }
    }
  }
  return {
    'begin': begin,
    'end': end
  }
}

function getScale(begin, end) {
  const scales = [
    { start: 0, end: 5, scale: 1*MINUTE },
    { start: 5, end: 10, scale: 2*MINUTE },
    { start: 10, end: 30, scale: 5*MINUTE },
    { start: 30, end: 90, scale: 10*MINUTE },
    { start: 90, end: 180, scale: 30*MINUTE },
    { start: 3*60, end: 6*60, scale: 60*MINUTE },
    { start: 6*60, end: 12*60, scale: 120*MINUTE },
    { start: 12*60, end: 48*60, scale: 240*MINUTE },
  ];

  const minutes = (end.getTime() - begin.getTime()) / (60*1000);
  let scale = 480*MINUTE;
  for (let i = 0; i < scales.length; i++) {
    let s = scales[i];
    if (minutes >= s.start && minutes < s.end) {
      scale = s.scale;
      break;
    }
  }

  return scale;
}

function roundToScale(t, scale) {
  t.setSeconds(0, 0);

  const midnight = new Date(t.getTime());
  midnight.setHours(0, 0, 0, 0);

  const sinceMidnight = t.getTime() - midnight.getTime();
  t.setTime(t.getTime() - (sinceMidnight % scale));

  return t;
}

let calculateTimelineSettings = function(cons, width, padding) {
  const beginAndEnd = getBeginAndEnd(cons);
  const begin = beginAndEnd.begin;
  const end = beginAndEnd.end;
  const scale = getScale(begin, end);
  return {
    'begin': begin,
    'end': end,
    'scale': scale,
    'start': roundToScale(new Date(begin.getTime()), scale),
    'totalCuts': Math.ceil((end.getTime() - begin.getTime()) / scale),
    'timeToXIntercept': function(t) {
      const period = end.getTime() - begin.getTime();
      const until_t = t.getTime() - begin.getTime();
      const percent = until_t / period;
      return padding + (width - padding * 2) * percent;
    }
  }
}

export default calculateTimelineSettings;