module Widgets.Map exposing (Model, Msg, init, view, update, subscriptions)

import Html exposing (Html, Attribute, div, text)
import Html.Attributes exposing (..)
import Port exposing (..)
import Math.Vector2 exposing (Vec2)
import Math.Vector3 exposing (Vec3, vec3)
import Math.Matrix4 exposing (Mat4, scale, translate, makeOrtho2D)
import WebGL exposing (..)
import Task exposing (..)
import Html exposing (Html)


-- MODEL


type alias Vertex =
    { coordinate : Vec3 }


mesh : Drawable Vertex
mesh =
    let
        ( x1, y1 ) =
            latLngToWorldCoord 49 9

        ( x2, y2 ) =
            latLngToWorldCoord 49.878 8.6512

        ( x3, y3 ) =
            latLngToWorldCoord 49.8728 8.61

        ( x4, y4 ) =
            latLngToWorldCoord 49.8728 8.65
    in
        Points
            [ Vertex (vec3 x1 y1 0)
            , Vertex (vec3 x2 y2 0)
            , Vertex (vec3 x3 y3 0)
            , Vertex (vec3 x4 y4 0)
            ]


type alias Model =
    { map : Map
    , texture : Maybe WebGL.Texture
    }


init : ( Model, Cmd Msg )
init =
    ( { map =
            { width = 0
            , height = 0
            , scale = 0
            , zoom = 0
            , north = 0
            , west = 0
            }
      , texture = Nothing
      }
    , mapInit "map"
    )



-- UPDATE


type Msg
    = MapLoaded String
    | MapUpdate Map
    | TextureError WebGL.Error
    | TextureLoaded WebGL.Texture


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        MapLoaded _ ->
            ( model
            , Task.perform TextureError
                TextureLoaded
                (WebGL.loadTexture "circle.png")
            )

        MapUpdate m' ->
            ( { model | map = m' }, Cmd.none )

        TextureError err ->
            ( model, Cmd.none )

        TextureLoaded texture ->
            ( { model | texture = Just texture }, Cmd.none )



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ mapLoaded MapLoaded
        , mapUpdate MapUpdate
        ]



-- VIEW


overlay : List (Html.Attribute Msg) -> Model -> Html Msg
overlay attributes model =
    (case model.texture of
        Nothing ->
            []

        Just tex ->
            [ render vertexShader
                fragmentShader
                mesh
                { texture = tex
                , perspective = perspective model
                , zoom = model.map.zoom
                }
            ]
    )
        |> WebGL.toHtmlWith
            [ Enable Blend
            , Disable DepthTest
            , BlendFunc ( SrcAlpha, OneMinusSrcAlpha )
            ]
            attributes


view : Model -> Html Msg
view model =
    div [ id "map" ] [ overlay [ class "leaflet-overlay" ] model ]


perspective : Model -> Mat4
perspective { map } =
    makeOrtho2D 0.0 map.width map.height 0.0
        |> scale (vec3 map.scale map.scale map.scale)
        |> translate (vec3 -(map.west / map.scale) -(map.north / map.scale) 0)



-- SHADERS


vertexShader : Shader { attr | coordinate : Vec3 } { unif | perspective : Mat4, zoom : Float } { vcoord : Vec2 }
vertexShader =
    [glsl|
attribute vec3 coordinate;
uniform mat4 perspective;
uniform float zoom;
varying vec2 vcoord;

void main() {
    gl_Position = perspective * vec4(coordinate, 1.0);
    gl_PointSize = zoom;
    vcoord = coordinate.xy;
}
|]


fragmentShader : Shader {} { u | texture : Texture } { vcoord : Vec2 }
fragmentShader =
    [glsl|
precision mediump float;
uniform sampler2D texture;
varying vec2 vcoord;

void main () {
    gl_FragColor = texture2D(texture, vec2(gl_PointCoord.x, gl_PointCoord.y));
}
|]



-- UTIL


latLngToWorldCoord : Float -> Float -> ( Float, Float )
latLngToWorldCoord lat lng =
    let
        initialResolution =
            2 * pi * 6378137 / 256

        originShift =
            pi * 6378137

        mx =
            lng * originShift / 180

        my1 =
            (logBase e (tan ((90 + lat) * pi / 360))) / (pi / 180)

        my =
            my1 * originShift / 180

        x =
            (mx + originShift) / initialResolution

        y =
            (my + originShift) / initialResolution
    in
        ( x, 256 - y )
