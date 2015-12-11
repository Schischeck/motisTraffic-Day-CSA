import { Matrix } from './Matrix';
import { TextureRenderer } from './TextureRenderer';
import { ShapeRenderer } from './ShapeRenderer';

export class MapRenderer {

  constructor(canvas, map) {
    this.canvas = canvas;
    this.map = map;
    this.pixelsToWebGLMatrix = new Matrix();
    this.mapMatrix = new Matrix();
    this.layers = [];

    this.gl = this.initGL(canvas);
    this.onResize();
    this.textureRenderer = new TextureRenderer(this.gl);
    this.shapeRenderer = new ShapeRenderer(this.gl);
    window.addEventListener('resize', this.onResize.bind(this));
    window.requestAnimationFrame(this.update.bind(this));
    this.map.on('zoomstart', this.renderLayers.bind(this));
    this.map.on('move', this.renderLayers.bind(this));
    this.map.on('resize', this.renderLayers.bind(this));
  }

  onResize() {
    this.canvas.width = window.innerWidth;
    this.canvas.height = window.innerHeight;
    let width = this.canvas.width;
    let height = this.canvas.height;
    this.gl.viewport(0, 0, width, height);
    this.pixelsToWebGLMatrix.fromArray([
      2 / this.canvas.width,0,0,0, 
      0,-2 / this.canvas.height,0,0,
      0,0,0,0,
      -1,1,0,1
    ]);
  }

  initGL(canvas) {
    let gl;
    try {
      gl = canvas.getContext("webgl") || canvas.getContext("experimental-webgl");
      let width = canvas.width;
      let height = canvas.height;
      gl.viewportWidth = width;
      gl.viewportHeight = height;
      gl.viewport(0, 0, width, height);
    } catch (e) {}
    if (!gl) {
      alert("Unable to initialize WebGL. Your browser may not support it.");
      gl = null;
    }
    return gl;
  }

  update() {
    this.updateLayers();
    this.render();
  }

  render() {
    this.renderLayers();
    window.requestAnimationFrame(this.update.bind(this));
  }

  getMatrix() {
    // copy pixel->webgl matrix
    this.mapMatrix.set(this.pixelsToWebGLMatrix);
    // Scale to current zoom (worldCoords * 2^zoom)
    let scale = Math.pow(2, this.map.getZoom());
    this.mapMatrix.scale(scale, scale);
    let bounds = this.map.getBounds();
    let topleft = new L.LatLng(bounds.getNorth(), bounds.getWest());
    let offset = this.map.project(topleft, 0);
    this.mapMatrix.translate(-offset.x, -offset.y);
    this.mapMatrix.zoom = this.map.getZoom() * 0.5;
    return this.mapMatrix;
  }

  renderLayers() {
    let matrix = this.getMatrix();
    let gl = this.gl;
    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.enable(gl.BLEND);
    gl.disable(gl.DEPTH_TEST);
    gl.blendFunc(gl.ONE_MINUS_DST_ALPHA, gl.DST_ALPHA);
    this.layers.forEach(layer => {
      let buffers = layer.getLayerBuffers();
      let textures = layer.getTextures();
      let metaData = layer.getMetaData();
      let entities = layer.getEntities();
      for (let i = 0; i < buffers.length; i++) {
        if (textures[i] != "LINES") {
          this.textureRenderer.render(gl, buffers[i], textures[i], matrix, metaData[i] && metaData[i]['zoom'] ? metaData[i].zoom : matrix.zoom);
        } else {
          this.shapeRenderer.render(gl, buffers[i], gl.LINES, metaData[i], matrix);
        }
      }
    });
  }

  updateLayers() {
    this.layers.forEach(layer => {
      layer.update();
      if (layer.dirty) {
        layer.loadBuffers(this.gl);
      }
    });
  }

  add(layer) {
    this.layers.unshift(layer);
  }

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
