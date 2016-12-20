module Widgets.Map.Stations exposing (..)

import Widgets.Map.RailVizModel exposing (..)
import Math.Vector2 as Vector2 exposing (Vec2, vec2)
import Math.Vector3 as Vector3 exposing (Vec3, vec3)
import Math.Matrix4 exposing (Mat4, scale, translate, makeOrtho2D)
import WebGL exposing (..)


-- MODEL


type alias Vertex =
    { aCoords : Vec2
    , aPickColor : Vec3
    }


mesh : List RVStation -> Drawable Vertex
mesh stations =
    Points (List.map stationVertex stations)


stationVertex : RVStation -> Vertex
stationVertex station =
    { aCoords = station.pos
    , aPickColor = station.pickColor
    }



-- SHADERS


vertexShader : Shader { attr | aCoords : Vec2 } { unif | uPerspective : Mat4, uZoom : Float } {}
vertexShader =
    [glsl|
attribute vec2 aCoords;
uniform mat4 uPerspective;
uniform float uZoom;

void main() {
    gl_Position = uPerspective * vec4(aCoords, 0.0, 1.0);
    gl_PointSize = uZoom;
}
|]


fragmentShader : Shader {} { u | uTexture : Texture } {}
fragmentShader =
    [glsl|
precision mediump float;
uniform sampler2D uTexture;

void main () {
    gl_FragColor = texture2D(uTexture, gl_PointCoord);
}
|]


offscreenVertexShader : Shader { attr | aCoords : Vec2, aPickColor : Vec3 } { unif | uPerspective : Mat4, uZoom : Float } { vPickColor : Vec3 }
offscreenVertexShader =
    [glsl|
precision highp float;
attribute vec2 aCoords;
attribute vec3 aPickColor;
uniform mat4 uPerspective;
uniform float uZoom;
varying vec3 vPickColor;

void main() {
    gl_Position = uPerspective * vec4(aCoords, 0.0, 1.0);
    gl_PointSize = uZoom;

    vPickColor = aPickColor;
}
|]


offscreenFragmentShader : Shader {} { u | uTexture : Texture } { vPickColor : Vec3 }
offscreenFragmentShader =
    [glsl|
precision highp float;
uniform sampler2D uTexture;
varying vec3 vPickColor;

void main () {
    vec4 tex = texture2D(uTexture, gl_PointCoord);
    if (tex.w == 0.0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        gl_FragColor = vec4(vPickColor, 1.0);
    }
}
|]
