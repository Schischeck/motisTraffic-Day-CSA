import RailVizStore from '../RailVizStore';

export class Layer {

  constructor(id) {
    this.dirty = true;
		this.static = false;
    this.name = id;
    this.layerBuffers = [];
    this.textures = [];
    this.entityBuffers = [];
    this.metaData = [];
  }

  update() {
		if (!this.static) {
    	this.entityBuffers.forEach(entities => {
      	entities.forEach(entity => {
         	entity.update();
        	if (entity.dirty) {
          	this.dirty = true;
        	}
      	});
   	 });
		}
  }

  loadBuffers(gl) {
    this.layerBuffers = [];
    for (let i = 0; i < this.entityBuffers.length; i++) {
      let data = [];
      this.entityBuffers[i].forEach(entity => {
				if (entity.isVisible()) {
        	data.push(entity.x);
        	data.push(entity.y);
				}
      });
      let buffer = gl.createBuffer();
      gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
      gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(data), gl.STATIC_DRAW);
      buffer.itemNum = data.length / 2;
      this.layerBuffers.push(buffer);
    }
    this.dirty = false;
  }

  getLayerBuffers() {
    return this.layerBuffers;
  }

  getTextures() {
    return this.textures;
  }

  getMetaData() {
    return this.metaData;
  }

  getEntities() {
		let entities = [];
    this.entityBuffers.forEach(buffer =>{
    	buffer.forEach(entity => entities.push(entity));
    });
		return entities;
  }

  add(entity) {
    let found = false;
    for (let i = 0; i < this.textures.length; i++) {
      if (entity.texture.id === this.textures[i].id) {
        this.entityBuffers[i].push(entity);
        found = true;
        break;
      }
    }
    if (!found) {
      this.textures.push(entity.texture);
      this.entityBuffers.push([
        entity
      ]);
    }
  }

  addAll(entities, texture, metaData) {
    this.textures.push(texture);
    this.entityBuffers.push(entities);
    this.metaData.push(metaData);
  }

}
