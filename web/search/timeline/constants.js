const isFirefox = typeof InstallTrigger !== 'undefined';
const isMS = document.documentMode || /Edge/.test(navigator.userAgent);

const RADIUS = 13;
const THICKNESS = 9;
const PADDING = 50;

const CUT_RADIUS = Math.cos(Math.asin((THICKNESS / 2) / RADIUS)) * RADIUS;
const X_OFFSET = CUT_RADIUS + RADIUS;
const MIN_LENGTH = CUT_RADIUS / 2;
const CIRCLE_WIDTH = (RADIUS + CUT_RADIUS) + (THICKNESS / 6.0);

const MOVE_LABEL_Y_OFFSET = ((isFirefox || isMS) ? 0 : -0.45) * THICKNESS;

const INFO_HOVER_OFFSET = (isFirefox || isMS) ? 8 : 20;
const INFO_HOVER_WIDTH = 280;
const INFO_HOVER_HEIGHT = 120;

const DETAIL_VIEW_CONNECTION_LENGTH = 100;

const constants = {
  RADIUS,
  THICKNESS,
  PADDING,
  CUT_RADIUS,
  X_OFFSET,
  MIN_LENGTH,
  CIRCLE_WIDTH,
  MOVE_LABEL_Y_OFFSET,
  INFO_HOVER_OFFSET,
  INFO_HOVER_WIDTH,
  INFO_HOVER_HEIGHT,
  DETAIL_VIEW_CONNECTION_LENGTH
}

export default constants;
