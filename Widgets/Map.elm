module Widgets.Map exposing (Model, Msg, init, view, update, subscriptions)

import Html exposing (Html, Attribute, div, text)
import Html.Attributes exposing (..)
import Port exposing (..)
import Math.Vector3 exposing (..)
import Math.Matrix4 exposing (..)
import WebGL exposing (..)
import Html exposing (Html)


-- MODEL


type alias Vertex =
    { coordinate : Vec3 }


mesh : Drawable Vertex
mesh =
    let
        ( x1, y1 ) =
            latLngToWorldCoord 49.8728 8.6512

        ( x2, y2 ) =
            latLngToWorldCoord 50.1109 8.6821

        ( x3, y3 ) =
            latLngToWorldCoord 49.9807 9.1356
    in
        Triangle
            [ ( Vertex (vec3 x1 y1 0)
              , Vertex (vec3 x2 y2 0)
              , Vertex (vec3 x3 y3 0)
              )
            ]


type alias Model =
    { width : Float
    , height : Float
    , zoomFactor : Float
    , scale : Float
    , north : Float
    , west : Float
    }


init : ( Model, Cmd Msg )
init =
    ( { width = 0
      , height = 0
      , zoomFactor = 0
      , scale = 0
      , north = 0
      , west = 0
      }
    , mapInit "map"
    )



-- UPDATE


type Msg
    = Ready
    | Loaded String
    | Zoom Float
    | ResizeWidth Float
    | ResizeHeight Float
    | North Float
    | West Float


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        Ready ->
            ( model, Cmd.none )

        Loaded mapId ->
            ( model, Cmd.none )

        Zoom zoomFactor ->
            ( { model
                | zoomFactor = zoomFactor
                , scale = 2 ^ zoomFactor
              }
            , Cmd.none
            )

        ResizeWidth w ->
            ( { model | width = w }, Cmd.none )

        ResizeHeight h ->
            ( { model | height = h }, Cmd.none )

        North n ->
            ( { model | north = snd (latLngToWorldCoord n 0) }, Cmd.none )

        West w ->
            ( { model | west = fst (latLngToWorldCoord 0 w) }, Cmd.none )



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ mapLoaded Loaded
        , mapResizeWidth ResizeWidth
        , mapResizeHeight ResizeHeight
        , mapZoom Zoom
        , mapNorth North
        , mapWest West
        ]



-- VIEW


view : Model -> Html Msg
view model =
    div [ id "map" ]
        [ WebGL.toHtml
            [ width (round model.width)
            , height (round model.height)
            , id "glCanvas"
            ]
            [ render vertexShader
                fragmentShader
                mesh
                { perspective = perspective model
                }
            ]
        ]


perspective : Model -> Mat4
perspective model =
    makeOrtho2D 0.0 model.width model.height 0.0
        |> Math.Matrix4.scale (vec3 model.scale model.scale model.scale)
        |> Math.Matrix4.translate (vec3 -model.west -model.north 0)



-- SHADERS


vertexShader : Shader { attr | coordinate : Vec3 } { unif | perspective : Mat4 } {}
vertexShader =
    [glsl|
attribute vec3 coordinate;
uniform mat4 perspective;

void main() {
    gl_Position = perspective * vec4(coordinate, 1.0);
}
|]


fragmentShader : Shader {} u {}
fragmentShader =
    [glsl|
precision mediump float;
void main () {
    gl_FragColor = vec4(1.0, 0.0, 0.0, 0.2);
}
|]



-- UTIL


latLngToWorldCoord : Float -> Float -> ( Float, Float )
latLngToWorldCoord lat lng =
    let
        initialResolution =
            2 * pi * 6378137 / 256

        originShift =
            2 * pi * 6378137 / 2

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
