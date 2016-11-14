module Widgets.Map exposing (Model, Msg, init, view, update, subscriptions)

import Html exposing (Html, Attribute, div, text)
import Html.Attributes exposing (..)
import Port exposing (..)
import Math.Vector3 exposing (Vec3, vec3)
import Math.Matrix4 exposing (Mat4, scale, translate, makeOrtho2D)
import WebGL exposing (..)
import Time exposing (Time)
import AnimationFrame
import Task exposing (..)
import Html exposing (Html)


-- MODEL


type alias Vertex =
    { coordinate : Vec3 }


( x1, y1 ) =
    latLngToWorldCoord 49.8728 8.61
( x2, y2 ) =
    latLngToWorldCoord 49.8728 8.65
mesh : Float -> Drawable Vertex
mesh p =
    let
        ax =
            x1 + p * (x2 - x1)

        ay =
            y1 + p * (y2 - y1)

        bx =
            x2 + p * (x1 - x2)

        by =
            y2 + p * (y1 - y2)
    in
        Points
            [ Vertex (vec3 ax ay 0)
            , Vertex (vec3 bx by 0)
            ]


type alias Model =
    { map : Map
    , texture : Maybe WebGL.Texture
    , time : Time
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
      , time = 0.0
      }
    , mapInit "map"
    )



-- UPDATE


type Msg
    = Animate Time
    | MapLoaded String
    | MapUpdate Map
    | TextureError WebGL.Error
    | TextureLoaded WebGL.Texture


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        Animate t ->
            ( { model | time = t }, Cmd.none )

        MapLoaded _ ->
            ( model
            , Task.perform TextureError
                TextureLoaded
                (WebGL.loadTexture "circle.png")
            )

        MapUpdate m_ ->
            ( { model | map = m_ }, Cmd.none )

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
        , AnimationFrame.times Animate
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
                (mesh (progress model))
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


progress : Model -> Float
progress model =
    toFloat (round model.time % 60000) / 60000


perspective : Model -> Mat4
perspective { map } =
    makeOrtho2D 0.0 map.width map.height 0.0
        |> scale (vec3 map.scale map.scale map.scale)
        |> translate (vec3 -(map.west / map.scale) -(map.north / map.scale) 0)



-- SHADERS


vertexShader : Shader { attr | coordinate : Vec3 } { unif | perspective : Mat4, zoom : Float } {}
vertexShader =
    [glsl|
attribute vec3 coordinate;
uniform mat4 perspective;
uniform float zoom;

void main() {
    gl_Position = perspective * vec4(coordinate, 1.0);
    gl_PointSize = zoom;
}
|]


fragmentShader : Shader {} { u | texture : Texture } {}
fragmentShader =
    [glsl|
precision mediump float;
uniform sampler2D texture;

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
