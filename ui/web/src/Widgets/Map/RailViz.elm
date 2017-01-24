module Widgets.Map.RailViz
    exposing
        ( Msg(SetTimeOffset)
        , Model
        , init
        , view
        , update
        , subscriptions
        , mapId
        )

import Widgets.Map.Port as Port exposing (..)
import Html exposing (Html, Attribute, div, text, span)
import Html.Attributes exposing (id, class, classList, style)
import Time exposing (Time)
import Date exposing (Date)
import Task exposing (..)
import Html exposing (Html)
import Maybe.Extra exposing (isJust, isNothing)
import Util.Core exposing ((=>))
import Util.DateFormat exposing (formatTime, formatDateTimeWithSeconds)
import Localization.Base exposing (..)


-- MODEL


type alias Model =
    { mapInfo : MapInfo
    , time : Time
    , systemTime : Time
    , timeOffset : Float
    , mouseX : Int
    , mouseY : Int
    , hoveredTrain : Maybe RVTrain
    , hoveredStation : Maybe String
    }


type alias RVTrain =
    { names : List String
    , departureTime : Time
    , arrivalTime : Time
    , scheduledDepartureTime : Time
    , scheduledArrivalTime : Time
    , departureStation : String
    , arrivalStation : String
    }


init : String -> ( Model, Cmd Msg )
init remoteAddress =
    { mapInfo =
        { scale = 0
        , zoom = 0
        , pixelBounds = { north = 0, west = 0, width = 0, height = 0 }
        , geoBounds = { north = 0, west = 0, south = 0, east = 0 }
        , railVizBounds = { north = 0, west = 0, south = 0, east = 0 }
        }
    , time = 0
    , systemTime = 0
    , timeOffset = 0
    , mouseX = 0
    , mouseY = 0
    , hoveredTrain = Nothing
    , hoveredStation = Nothing
    }
        ! [ Task.perform SetTime Time.now
          , mapInit mapId
          ]



-- TODO!


filteredZoomMin : number
filteredZoomMin =
    13



-- UPDATE


type Msg
    = MapUpdate MapInfo
    | SetTime Time
    | SetTimeOffset Float


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        MapUpdate m_ ->
            let
                model_ =
                    { model | mapInfo = m_ }
            in
                model_ ! []

        SetTime time ->
            { model
                | systemTime = time
                , time = time + model.timeOffset
            }
                ! []

        SetTimeOffset offset ->
            { model
                | timeOffset = offset
                , time = model.systemTime + offset
            }
                ! []



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ mapUpdate MapUpdate
        , Time.every Time.second SetTime
        ]



-- VIEW


mapId : String
mapId =
    "map"


view : Localization -> Model -> Html Msg
view locale model =
    div [ class "map-container" ]
        [ div [ class "inner-map-container" ]
            [ div [ id mapId ]
                [ Html.canvas
                    [ classList
                        [ "railviz-overlay" => True
                        , "leaflet-zoom-hide" => True
                        , "train-hover" => (isJust model.hoveredTrain || isJust model.hoveredStation)
                        ]
                    ]
                    []
                ]
            , railVizTooltip model
            , simulationTimeOverlay locale model
            ]
        ]


railVizTooltip : Model -> Html Msg
railVizTooltip model =
    case model.hoveredTrain of
        Just train ->
            railVizTrainTooltip model train

        Nothing ->
            case model.hoveredStation of
                Just station ->
                    railVizStationTooltip model station

                Nothing ->
                    div [ class "railviz-tooltip hidden" ] []


railVizTooltipWidth : number
railVizTooltipWidth =
    240


railVizTrainTooltip : Model -> RVTrain -> Html Msg
railVizTrainTooltip model train =
    let
        ttWidth =
            railVizTooltipWidth

        ttHeight =
            55

        margin =
            20

        x =
            (model.mouseX - (ttWidth // 2))
                |> max margin
                |> min (floor model.mapInfo.pixelBounds.width - ttWidth - margin)

        below =
            model.mouseY + margin + ttHeight < floor model.mapInfo.pixelBounds.height

        y =
            if below then
                model.mouseY + margin
            else
                (floor model.mapInfo.pixelBounds.height) - (model.mouseY - margin)

        yAnchor =
            if below then
                "top"
            else
                "bottom"

        trainName =
            String.join " / " train.names

        schedDep =
            Date.fromTime train.scheduledDepartureTime

        schedArr =
            Date.fromTime train.scheduledArrivalTime

        depDelay =
            ceiling ((train.departureTime - train.scheduledDepartureTime) / 60000)

        arrDelay =
            ceiling ((train.arrivalTime - train.scheduledArrivalTime) / 60000)
    in
        div
            [ class "railviz-tooltip train visible"
            , style
                [ yAnchor => (toString y ++ "px")
                , "left" => (toString x ++ "px")
                ]
            ]
            [ div [ class "transport-name" ] [ text trainName ]
            , div [ class "departure" ]
                [ span [ class "station" ] [ text train.departureStation ]
                , div [ class "time" ]
                    [ span [ class "schedule" ] [ text (formatTime schedDep) ]
                    , delayView depDelay
                    ]
                ]
            , div [ class "arrival" ]
                [ span [ class "station" ] [ text train.arrivalStation ]
                , div [ class "time" ]
                    [ span [ class "schedule" ] [ text (formatTime schedArr) ]
                    , delayView arrDelay
                    ]
                ]
            ]


railVizStationTooltip : Model -> String -> Html Msg
railVizStationTooltip model stationName =
    let
        ttWidth =
            railVizTooltipWidth

        ttHeight =
            22

        margin =
            20

        x =
            (model.mouseX - (ttWidth // 2))
                |> max margin
                |> min (floor model.mapInfo.pixelBounds.width - ttWidth - margin)

        below =
            model.mouseY + margin + ttHeight < floor model.mapInfo.pixelBounds.height

        y =
            if below then
                model.mouseY + margin
            else
                (floor model.mapInfo.pixelBounds.height) - (model.mouseY - margin)

        yAnchor =
            if below then
                "top"
            else
                "bottom"
    in
        div
            [ class "railviz-tooltip station visible"
            , style
                [ yAnchor => (toString y ++ "px")
                , "left" => (toString x ++ "px")
                ]
            ]
            [ div [ class "station-name" ] [ text stationName ] ]


simulationTimeOverlay : Localization -> Model -> Html Msg
simulationTimeOverlay locale model =
    let
        simDate =
            Date.fromTime model.time
    in
        div
            [ class "sim-time-overlay" ]
            [ text (formatDateTimeWithSeconds locale.dateConfig simDate) ]


delayView : Int -> Html Msg
delayView minutes =
    let
        delayType =
            if minutes > 0 then
                "delay pos-delay"
            else
                "delay neg-delay"

        delayText =
            if minutes >= 0 then
                "+" ++ (toString minutes)
            else
                toString minutes
    in
        span [ class delayType ] [ text delayText ]
