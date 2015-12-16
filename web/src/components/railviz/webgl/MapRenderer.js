import {Matrix} from './Matrix';
import {TextureRenderer} from './TextureRenderer';
import {ShapeRenderer} from './ShapeRenderer';

export class MapRenderer {

  constructor(canvas, map) {
    this.gl = this._initGL(canvas);
    this.textureRenderer = new TextureRenderer(this.gl);
    this.shapeRenderer = new ShapeRenderer(this.gl);
    this.canvas = canvas;
    this.map = map;
    this.pixelsToWebGLMatrix = new Matrix();
    this.mapMatrix = new Matrix();
    this.layers = [];
    this._onResize();
    this._setupEvents();
  }

  _setupEvents() {
    window.addEventListener('resize', this._onResize.bind(this));
    window.requestAnimationFrame(this._update.bind(this));
    this.map.on('zoomstart', this._renderLayers.bind(this));
    this.map.on('move', this._renderLayers.bind(this));
    this.map.on('resize', this._renderLayers.bind(this));
  }

  _centerOn() {
    if (this.followEntity) {
      let latlng = this.map.unproject(
          new L.Point(this.followEntity.x, this.followEntity.y),0);
      let zoom_ = this.map.getZoom();
      this.map.setView(latlng, zoom_,
                       {animate : false});
    }
  }

	centerOn(entity){
		if(entity){
			this.followEntity = entity;
			this.follow = true;
		}else{
			this.follow = false;
		}
	}
	
  project(position) {
    let matrix = this._getMatrix();
    let pos = matrix.multiply([ position.x, position.y, 1, 1 ]);
    return {x : pos[0], y : pos[1]};
  }

  _onResize() {
    this.canvas.width = window.innerWidth;
    this.canvas.height = window.innerHeight;
    let width = this.canvas.width;
    let height = this.canvas.height;
    this.gl.viewport(0, 0, width, height);
    this.pixelsToWebGLMatrix.fromArray([
      2 / this.canvas.width,
      0,
      0,
      0,
      0,
      -2 / this.canvas.height,
      0,
      0,
      0,
      0,
      0,
      0,
      -1,
      1,
      0,
      1
    ]);
  }

  _initGL(canvas) {
    let gl;
    try {
      gl =
          canvas.getContext("webgl") || canvas.getContext("experimental-webgl");
      let width = canvas.width;
      let height = canvas.height;
      gl.viewportWidth = width;
      gl.viewportHeight = height;
      gl.viewport(0, 0, width, height);
    } catch (e) {
    }
    if (!gl) {
      alert("Unable to initialize WebGL. Your browser may not support it.");
      gl = null;
    }
    return gl;
  }

  _update() {
    this._updateLayers();
    this._render();
    if (this.follow) {
      this._centerOn();
    }
  }

  _render() {
    this._renderLayers();
    window.requestAnimationFrame(this._update.bind(this));
  }

  _getMatrix() {
    // copy pixel->webgl matrix
    this.mapMatrix.set(this.pixelsToWebGLMatrix);
    // Scale to current zoom (worldCoords * 2^zoom)
    let scale = Math.pow(2, this.map.getZoom());
    this.mapMatrix.scale(scale, scale);
    let bounds = this.map.getBounds();
    let topleft = new L.LatLng(bounds.getNorth(), bounds.getWest());
    let offset = this.map.project(topleft, 0);
    this.mapMatrix.translate(-offset.x, -offset.y);
    let zoom = this.map.getZoom();
    this.mapMatrix.zoom = (zoom * zoom) * 0.035;
    return this.mapMatrix;
  }

  _renderLayers() {
    let matrix = this._getMatrix();
    let gl = this.gl;
    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.enable(gl.BLEND);
    gl.disable(gl.DEPTH_TEST);
    gl.blendFunc(gl.ONE_MINUS_DST_ALPHA, gl.DST_ALPHA);
    this.layers.forEach(layer => {
      let buffers = layer.getLayerBuffers();
      let textures = layer.getTextures();
      let metaData = layer.getMetaData();
      for (let i = 0; i < buffers.length; i++) {
        if (textures[i] != "LINES") {
          this.textureRenderer.render(gl, buffers[i], textures[i], matrix,
                                      metaData[i] && metaData[i]['zoom']
                                          ? metaData[i].zoom
                                          : matrix.zoom);
        } else {
          this.shapeRenderer.render(gl, buffers[i], gl.LINES, metaData[i],
                                    matrix);
        }
      }
    });
  }

  _updateLayers() {
    this.layers.forEach(layer => {
      layer.update();
      if (layer.dirty) {
        layer.loadBuffers(this.gl);
      }
    });
  }

  add(layer) { this.layers.push(layer); }

  remove(id) {
    let index;
    let found = false;
    for (let i = 0; i < this.layers.length; i++) {
      if (this.layers[i].name === id) {
        index = i;
        found = true;
      }
    }
    if (found)
      this.layers.splice(index, 1);
  }
	
	getLayers(){
		return this.layers;
	}

  createTextureFromCanvas(cv) {
    let gl = this.gl;
    let tex = gl.createTexture();
    gl.bindTexture(gl.TEXTURE_2D, tex);
    gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, cv);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
    gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
    if (cv.id) {
      tex.id = cv.id;
    }
    return tex;
  }
}