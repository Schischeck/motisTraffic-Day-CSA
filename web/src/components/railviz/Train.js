import TimeStore from './TimeStore';

export class Train {

  constructor(startTime, endTime, startStation, endStation, routeId, path = [], startTimeDelay = 0, endTimeDelay = 0) {
    this.startTime = startTime;
    this.endTime = endTime;
    this.startStation = startStation;
    this.endStation = endStation;
    this.routeId = routeId;
    this.path = path;
    this.startTimeDelay = startTimeDelay;
    this.endTimeDelay = endTimeDelay;
    let p = this._currentPosition(new Date());
    this.x = p.x;
    this.y = p.y;
    this.updatable = true;
  }

	isVisible() {
		let currentTime = TimeStore.getTime();
    if (currentTime < this.startTime || currentTime > this.endTime)
      return false;
    return true;
	}

  delayClass() {
    if (this.endTimeDelay <= 2)
      return 0;
    if (this.endTimeDelay <= 5)
      return 1;
    if (this.endTimeDelay <= 10)
      return 2;
    if (this.endTimeDelay <= 30)
      return 3;
    if (this.endTimeDelay > 30)
      return 4;
  }

  update() {
    let p = this._currentPosition(TimeStore.getTime());
    this.x = p.x;
    this.y = p.y;
    this.dirty = true;
  }

  _currentPosition(currentTime) {
    let maxDX = Math.abs(this.startStation.x - this.endStation.x);
    let maxDY = Math.abs(this.startStation.y - this.endStation.y);
    let driven = Math.fround(currentTime - this.startTime) / (this.endTime - this.startTime);
    if (this.startStation.x > this.endStation.x)
      maxDX *= -1;
    if (this.startStation.y > this.endStation.y)
      maxDY *= -1;
    return {
      x: Math.fround(this.startStation.x + (maxDX * driven)),
      y: Math.fround(this.startStation.y + (maxDY * driven))
    };
  }
}
