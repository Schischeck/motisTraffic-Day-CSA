import {Shader} from './Shader';
import {Matrix} from './Matrix';
import RailVizStore from '../RailVizStore';
import {Station} from '../Station';
import TimeStore from '../TimeStore';
import RailVizAlltraReq from '../../../Messages/RailVizAlltraReq';
import Server from '../../../Server'; 
import Dispatcher from '../../../Dispatcher';

export class WebGLLayer {
    constructor(canvas, map) {
		this.map = map;
		this.trainsGLBuffers = [null, null, null, null, null];
		this.trainsGLBuffersLen = [-1,-1,-1,-1,-1];
		this.tracksGLBuffer = null;
		this.isReady = false;
		this.pixelsToWebGLMatrix = new Matrix();
		this.mapMatrix = new Matrix();
    this.gl = this.initGL(canvas);
		this.initTextures();
		window.requestAnimationFrame(this.update.bind(this));
		this.setupEvents();
		this.interval = setInterval(this._tick, 1000);
    }
		
	    
	
		_tick(){
			Dispatcher.dispatch({
				content_type: 'tick'
			});
		}

    compileShaderProgram(vertexShaderSRC, fragmentShaderSRC) {
        let program = null;
        let vertexShader = this.gl.createShader(this.gl.VERTEX_SHADER);
        this.gl.shaderSource(vertexShader, vertexShaderSRC);
        this.gl.compileShader(vertexShader);
        let fragmentShader = this.gl.createShader(this.gl.FRAGMENT_SHADER);
        this.gl.shaderSource(fragmentShader, fragmentShaderSRC);
        this.gl.compileShader(fragmentShader);
        // link shaders to create our program
        program = this.gl.createProgram();
        this.gl.attachShader(program, vertexShader);
        this.gl.attachShader(program, fragmentShader);
        this.gl.linkProgram(program);
        if (!this.gl.getProgramParameter(program, this.gl.LINK_STATUS)) {
            console.error(this.gl.getProgramInfoLog(program));
        }
        return program;
    }
	
	initTextures(){
		this.stationTexture = this.gl.createTexture();
		this.stationTexture.image = new Image();
		this.stationTexture.image.src = '';
		this.stationTexture.image.onload = () => 
		{
			this.gl.bindTexture( this.gl.TEXTURE_2D, this.stationTexture );
			this.gl.texImage2D( this.gl.TEXTURE_2D, 0, this.gl.RGBA, this.gl.RGBA,
      	this.gl.UNSIGNED_BYTE, this.stationTexture.image );
			this.gl.texParameteri( this.gl.TEXTURE_2D, this.gl.TEXTURE_MAG_FILTER, this.gl.LINEAR );
			this.gl.texParameteri( this.gl.TEXTURE_2D, this.gl.TEXTURE_MIN_FILTER, this.gl.LINEAR_MIPMAP_NEAREST);
			this.gl.generateMipmap( this.gl.TEXTURE_2D );
	 		this.gl.bindTexture( this.gl.TEXTURE_2D, null );
		};
		this.trainTexture = {
			0: this.createCircleTexture( this.gl, 25, [71, 186, 0, 255], [59, 153, 0, 255], 8 ),
			1: this.createCircleTexture( this.gl, 50, [163, 255, 110, 255], [133, 225, 80, 255], 8 ),
			2: this.createCircleTexture( this.gl, 50, [255, 247, 0, 255], [225, 217, 0, 255], 8 ),
			3: this.createCircleTexture( this.gl, 50, [255, 191, 0, 255], [225, 161, 0, 255], 8 ),
			4: this.createCircleTexture( this.gl, 50, [255, 72, 0, 255], [225, 42, 0, 255], 8 )
		}
		this.program = this.compileShaderProgram(Shader.texture.vertex, Shader.texture.fragment);
		this.tracksProgram = this.compileShaderProgram(Shader.lines.vertex, Shader.lines.fragment);
	}

    initGL(canvas){
        let gl;
        try {
            // Try to grab the standard context. If it fails, fallback to experimental.
            gl = canvas.getContext("webgl") || canvas.getContext("experimental-webgl");
			let width = canvas.width;
			let height = canvas.height;
            gl.viewportWidth = width;
            gl.viewportHeight = height;
			gl.viewport(0, 0, width, height);
			this.pixelsToWebGLMatrix.fromArray([
				2/width, 0, 0, 0,
				0, -2/height, 0, 0, 
				0, 0, 0, 0, 
				-1, 1, 0, 1
			]);
        }
        catch(e) {}

        // If we don't have a GL context, give up now
        if (!gl) {
            alert("Unable to initialize WebGL. Your browser may not support it.");
            gl = null;
        }

        return gl;
    }
	
	update(){
		if(!this.stations){
			let s = RailVizStore.getStations();
			if(s){
				this.stations = s
				this.loadBuffers();
				this.sendRequest();
			}
		}
		if(this.stations){
			this.repaint();
			return;
		}
			window.requestAnimationFrame(this.update.bind(this));
	}
	
	setupEvents(){
		this.map.on('zoomstart', this.render.bind(this));
		this.map.on('move', this.render.bind(this));
		this.map.on('move', this.sendRequest.bind(this));
		this.map.on('resize', this.render.bind(this));
	}
	
	sendRequest(){
		let bounds = this.getBounds();
		let pp1 = this.project(bounds.p1);
		let pp2 = this.project(bounds.p2);
		let p1 = {x:Math.min(pp1.x,pp2.x), y:Math.min(pp1.y,pp2.y)};
		let p2 = {x:Math.max(pp1.x,pp2.x), y:Math.max(pp1.y,pp2.y)};
		let width = Math.abs(p1.x-p2.x);
		let height = Math.abs(p1.y-p2.y);
		p1.x = p1.x-width;
		p1.y = p1.y-height;
		if( p1.x < 0 ) p1.x = 0;
		if( p1.y < 0 ) p1.y = 0;
		p2.x = p2.x+width;
		p2.y = p2.y+height
		let ll1 = this.unproject(p1);
		let ll2 = this.unproject(p2);
		let time = Math.round((TimeStore.getTime() / 1000));
		this._sendMessage(new RailVizAlltraReq(ll1,ll2,time));
	}
	
	_sendMessage(message, componentId) {
	    Server.sendMessage(message).then((res) => {
	      Dispatcher.dispatch({
	        content_type: res.content_type,
	        content: res.content,
	      });
	    });
	}
		
	repaint(){
		this.render();
		window.requestAnimationFrame(this.repaint.bind(this));
	}

    render(){
       if(this.gl) {
        	this.gl.clear(this.gl.COLOR_BUFFER_BIT);
				this.renderStations();
				this.renderTrains();
		}
    }

	onResize(canvas){
		this.updateGL(canvas);
		this.render();
	}
	
	updateGL(canvas){
		canvas.width = window.innerWidth;
		canvas.height = window.innerHeight;
		let width = canvas.width;
		let height = canvas.height;
		this.gl.viewport(0, 0, width, height);
		// matrix which maps pixel coordinates to WebGL coordinates
		this.pixelsToWebGLMatrix.fromArray([2/width, 0, 0, 0, 0, -2/height, 0, 0,
				0, 0, 0, 0, -1, 1, 0, 1]);
	}
	
	loadBuffers(){
		if(this.stations && this.stations.length > 0){
			let stationPoints = [];
			this.stations.forEach(station =>{
				stationPoints.push(station.x);
				stationPoints.push(station.y);
			});
			//this._updateVisibleStations();
			let data = new Float32Array(stationPoints);
			this.stationBuffer = this.gl.createBuffer();
			this.gl.bindBuffer(this.gl.ARRAY_BUFFER, this.stationBuffer);
			this.gl.bufferData(this.gl.ARRAY_BUFFER, data, this.gl.STATIC_DRAW);
			this.stationBuffer.itemNum = stationPoints.length/2;	
		}
	}
	
	renderStations(){
		let mapMatrix = this.getMapMatrix();
		let gl = this.gl;
		// render trains
		gl.useProgram(this.program);

		gl.enable(gl.BLEND);
		gl.disable(gl.DEPTH_TEST);
		gl.blendFunc( gl.ONE_MINUS_DST_ALPHA, gl.DST_ALPHA );
		if(!this.stations || (this.stations.length < 0 || !this.stationBuffer.itemNum > 0)){
			console.error('no stations to draw');
			return;
		}
		let attrLoc = gl.getAttribLocation(this.program, 'worldCoord');
		let matrixLoc = gl.getUniformLocation(this.program, 'mapMatrix');
		let scaleLoc = gl.getUniformLocation( this.program, 'zoom' );
		let zoom = this.getZoom();
		gl.uniform1f( scaleLoc, parseFloat(0.0009*(zoom*zoom*zoom)));
		gl.uniformMatrix4fv(matrixLoc, false, new Float32Array(mapMatrix.data));
		gl.bindBuffer(gl.ARRAY_BUFFER, this.stationBuffer);
		gl.enableVertexAttribArray(attrLoc);
		gl.vertexAttribPointer(attrLoc, 2, gl.FLOAT, false, 0, 0);
		gl.activeTexture(gl.TEXTURE0);
		gl.bindTexture( gl.TEXTURE_2D, this.stationTexture );
		gl.uniform1i( gl.getUniformLocation(this.program, 'texture'), 0 );
		gl.drawArrays( gl.POINTS, 0, this.stationBuffer.itemNum);
	}
	
	renderTrains(){
		let trains = RailVizStore.getTrains();
		if(trains){
			let mapMatrix = this.getMapMatrix();
			let gl = this.gl;
			this.loadTrainsToBuffer(trains);
			// render trains
			gl.useProgram(this.program);
			let matrixLoc = gl.getUniformLocation(this.program, 'mapMatrix');
			gl.uniformMatrix4fv( matrixLoc, false, mapMatrix.data );
		
			let scaleLoc = gl.getUniformLocation(this.program, 'zoom');
			gl.uniform1f( scaleLoc, parseFloat(this.getZoom()*2) );
		
			for( let i = this.trainsGLBuffers.length-1; i >= 0; i-- )
			{
				gl.bindBuffer( gl.ARRAY_BUFFER, this.trainsGLBuffers[i] );
				let attrLoc = gl.getAttribLocation( this.program, 'worldCoord' );
				gl.enableVertexAttribArray( attrLoc );
				gl.vertexAttribPointer( attrLoc, 2, gl.FLOAT, false, 0, 0 );
		
				gl.activeTexture(gl.TEXTURE0);
				gl.bindTexture( gl.TEXTURE_2D,this.trainTexture[i]);
				gl.uniform1i( gl.getUniformLocation(this.program, 'texture'), 0 );
		
				gl.drawArrays( gl.POINTS, 0, this.trainsGLBuffersLen[i] );
		}
		
		//render tracks
		gl.useProgram( this.tracksProgram );
		gl.bindBuffer( gl.ARRAY_BUFFER, this.tracksGLBuffer );
		let attrLoc = gl.getAttribLocation( this.tracksProgram, 'worldCoord' );
		gl.enableVertexAttribArray( attrLoc );
		gl.vertexAttribPointer( attrLoc, 2, gl.FLOAT, false, 0, 0 );
	
		matrixLoc = gl.getUniformLocation( this.tracksProgram, 'mapMatrix' );
		gl.uniformMatrix4fv( matrixLoc, false, mapMatrix.data );
		
		for( let i = 0; i < this.pathMeta.length; i++ )
		{
			let meta = this.pathMeta[i];
			gl.drawArrays( gl.LINES, meta.start, meta.size );
		}
	}
	}
	
	loadTrainsToBuffer(trains){
		let gl = this.gl;
		let xytrains = [
			new Array(),
			new Array(),
			new Array(),
			new Array(),
			new Array()
		];
		let xytracks = new Array();
		this.pathMeta = new Array();
		this.numDrivingTrains = 0;
		for(let i = 0; i < trains.length; i++)
		{
			if(trains[i].isDriving(TimeStore.getTime()))
			{
				let train = trains[i];
				let p = train.currentPosition(TimeStore.getTime());
				xytrains[0].push(p.x, p.y);
				if( train.path.length < 2 )
				{
					this.pathMeta.push({start:Math.round(xytracks.length/2),size:2});
					xytracks.push(Math.fround(train.startStation.x), Math.fround(train.startStation.y), 
					              Math.fround(train.endStation.x), Math.fround(train.endStation.y));
				} else
				{
					this.pathMeta.push({start:Math.round(xytracks.length/2),size:(train.path.length-1)*2});
					for( let i = 0; i < train.path.length-1; i++ )
					{
						xytracks.push(Math.fround(train.path[i].x), Math.fround(train.path[i].y), 
					              Math.fround(train.path[i+1].x), Math.fround(train.path[i+1].y));
					}
				}
				this.numDrivingTrains += 1;
			}
		}

		for( let i = 0; i < xytrains.length; i++ )
		{
			let data = new Float32Array( xytrains[i] );
			if(!this.trainsGLBuffers[i])
				this.trainsGLBuffers[i] = gl.createBuffer();
			this.trainsGLBuffersLen[i] = xytrains[i].length/2;
			gl.bindBuffer( gl.ARRAY_BUFFER, this.trainsGLBuffers[i] );
			gl.bufferData( gl.ARRAY_BUFFER, data, gl.STATIC_DRAW );	
		}
		
		let tracksData = new Float32Array( xytracks );
		if(!this.tracksGLBuffer)
			this.tracksGLBuffer = gl.createBuffer();
		gl.bindBuffer( gl.ARRAY_BUFFER, this.tracksGLBuffer );
		gl.bufferData( gl.ARRAY_BUFFER, tracksData, gl.STATIC_DRAW );
	}
	
	getMapMatrix( zoom_=false, topleft_=false )
	{
		// copy pixel->webgl matrix
		this.mapMatrix.set(this.pixelsToWebGLMatrix);
		// Scale to current zoom (worldCoords * 2^zoom)
		let scale = Math.pow(2,this.getZoom());
		this.mapMatrix.scale(scale,scale);
		let bounds = this.map.getBounds();
		let topleft = new L.LatLng(bounds.getNorth(), bounds.getWest());
		if( topleft_ )
			topleft = topleft;
		let offset = this.map.project(topleft,0);
		this.mapMatrix.translate(-offset.x, -offset.y);
		return this.mapMatrix;
	}
	
	createCircleTexture( gl, rad, fillColor, borderColor, borderThickness )
	{
		let cv = document.createElement('canvas');
		cv.width = rad*2 + borderThickness*2;
		cv.height = rad*2 + borderThickness*2;
		let ct2d = cv.getContext('2d');
		ct2d.beginPath();
		ct2d.arc(rad+borderThickness, rad+borderThickness, rad, 0, 2*Math.PI, false);
		ct2d.fillStyle = 'rgba('+fillColor[0]+','+fillColor[1]+','+fillColor[2]+','+fillColor[3]+')';
		ct2d.fill();
		ct2d.lineWidth = borderThickness;
		ct2d.strokeStyle = 'rgba('+borderColor[0]+','+borderColor[1]+','+borderColor[2]+','+borderColor[3]+')';
		ct2d.stroke();
		
		let texture = this.createTextureFromCanvas(gl, cv);
		gl.bindTexture( gl.TEXTURE_2D, texture );
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.LINEAR);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
		gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
		
		return texture;
	}
	
	createTextureFromCanvas( gl, cv )
	{
		let tex = gl.createTexture();
		gl.bindTexture( gl.TEXTURE_2D, tex );
		gl.texImage2D(gl.TEXTURE_2D, 0, gl.RGBA, gl.RGBA, gl.UNSIGNED_BYTE, cv);
		return tex;
	}
	
	project(latlng)
	{
		
		let ll = new L.LatLng(latlng.lat, latlng.lng);
		//let lp = L.Projection.SphericalMercator.project(ll);
		//let lp = this.crs.project(ll);
		let lp = this.map.project(ll, 0);
		let p = {x: lp.x, y: lp.y};
		return p;
	}
	
	unproject(p)
	{
		let lp = new L.Point(p.x, p.y);
		let ll = this.map.unproject(lp, 0);
		let latlng = {lat: ll.lat, lng: ll.lng};
		return latlng;
	}
	
	getZoom()
	{
		return this.map.getZoom();
	}
	
	getTopLeft()
	{
		let ll = this.map.getBounds().getNorthWest();
		let latlng = {lat: ll.lat, lng: ll.lng};
		return latlng;
	}
	
	getBounds()
	{
		let nw = this.map.getBounds().getNorthWest();
		let se = this.map.getBounds().getSouthEast();
		let b = {
			p1: {lat: nw.lat, lng: nw.lng},
			p2: {lat: se.lat, lng: se.lng}
		};
		return b;
	}
}
