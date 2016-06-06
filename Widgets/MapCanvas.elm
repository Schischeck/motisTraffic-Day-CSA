module Widgets.MapCanvas exposing (Model, Msg, init, view, update, subscriptions)

import Math.Vector3 exposing (..)
import Math.Matrix4 exposing (..)
import WebGL exposing (..)
import Html exposing (Html)
import AnimationFrame


-- Create a mesh with two triangles


type alias Vertex =
    { position : Vec3, color : Vec3 }


mesh : Drawable Vertex
mesh =
    Triangle
        [ ( Vertex (vec3 0 0 0) (vec3 1 0 0)
          , Vertex (vec3 1 1 0) (vec3 0 1 0)
          , Vertex (vec3 1 -1 0) (vec3 0 0 1)
          )
        ]


type alias Model =
    Float


type alias Msg =
    Float


init : ( Float, Cmd a )
init =
    ( 0, Cmd.none )


subscriptions : Model -> Sub Msg
subscriptions model =
    AnimationFrame.diffs Basics.identity


update : Msg -> Model -> ( Model, Cmd Msg )
update elapsed currentTime =
    ( elapsed + currentTime, Cmd.none )


view : List (Html.Attribute msg) -> Model -> Html msg
view attributes t =
    WebGL.toHtml attributes
        [ render vertexShader fragmentShader mesh { perspective = perspective (t / 1000) } ]


perspective : Float -> Mat4
perspective t =
    mul (makePerspective 45 1 0.01 100)
        (makeLookAt (vec3 (4 * cos t) 0 (4 * sin t)) (vec3 0 0 0) (vec3 0 1 0))



-- Shaders


vertexShader : Shader { attr | position : Vec3, color : Vec3 } { unif | perspective : Mat4 } { vcolor : Vec3 }
vertexShader =
    [glsl|
attribute vec3 position;
attribute vec3 color;
uniform mat4 perspective;
varying vec3 vcolor;
void main () {
    gl_Position = perspective * vec4(position, 1.0);
    vcolor = color;
}
|]


fragmentShader : Shader {} u { vcolor : Vec3 }
fragmentShader =
    [glsl|
precision mediump float;
varying vec3 vcolor;
void main () {
    gl_FragColor = vec4(vcolor, 1.0);
}
|]
