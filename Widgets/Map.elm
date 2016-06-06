module Widgets.Map exposing (Model, Msg, init, view, update, subscriptions)

import Html exposing (Html, Attribute, div, text)
import Html.Attributes exposing (..)
import Widgets.MapCanvas as MapCanvas
import Port exposing (..)


-- MODEL


type alias Model =
    { canvas : MapCanvas.Model
    }


init : ( Model, Cmd Msg )
init =
    let
        ( m, c ) =
            MapCanvas.init
    in
        ( Model m, Cmd.batch [ c, initMap "map" ] )



-- UPDATE


type Msg
    = MapContainerReady
    | MapLoaded String
    | CanvasUpdate MapCanvas.Msg


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        MapContainerReady ->
            ( model, Cmd.none )

        MapLoaded mapId ->
            ( model, Cmd.none )

        CanvasUpdate msg' ->
            let
                ( m, c ) =
                    MapCanvas.update msg' model.canvas
            in
                ( Model m, Cmd.map CanvasUpdate c )



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ mapLoaded MapLoaded
        , Sub.map CanvasUpdate (MapCanvas.subscriptions model.canvas)
        ]



-- VIEW


view : Model -> Html Msg
view model =
    div [ id "map" ]
        [ MapCanvas.view [ width 400, height 400, id "glCanvas" ] model.canvas ]
