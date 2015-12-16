

export class Station {
  constructor(id, lat, lng, x, y, name) {
    this.lat = lat;
    this.lng = lng;
    this.x = x;
    this.y = y;
    this.name = name;
		this.id = id;
  }

  update() {}
	
	isVisible() {return true;}
}
