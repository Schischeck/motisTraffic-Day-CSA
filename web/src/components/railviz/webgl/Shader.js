export var Shader = {
	texture: {
		vertex: `
			// TRAIN VERTEX SHADER __________________________________
			attribute vec4 worldCoord;

			uniform float zoom;
			uniform mat4 mapMatrix;

			void main() {
				// transform world coordinate by matrix uniform variable
				gl_Position = (mapMatrix * worldCoord);
				
				// a constant size for points, regardless of zoom level
				gl_PointSize = zoom;
			}
			//TRAIN VERTEX SHADER END________________________________
		`,
		fragment: `
			precision mediump float;

			uniform sampler2D texture;

			void main() {
				vec4 color = texture2D( texture, vec2(gl_PointCoord.x, gl_PointCoord.y) );
				gl_FragColor = color;
			}
		`
	},
	
	lines: {
		vertex: `
			// TRACK VERTEX SHADER __________________________________
			attribute vec4 worldCoord;

			uniform float zoom;
			uniform mat4 mapMatrix;

			void main() {
				// transform world coordinate by matrix uniform variable
				gl_Position = mapMatrix * worldCoord;
				
				// a constant size for points, regardless of zoom level
				gl_PointSize = 10.0;
			}
			//TRAIN VERTEX SHADER END________________________________
		`,
		fragment: `
			// TRACK FRAGMENT SHADER ________________________________
			precision mediump float;

			void main() {
				gl_FragColor = vec4(0.0,0.0,0.0,1.0);
			}
			// TRACK FRAGMENT SHADER END_____________________________
		`
	},
};