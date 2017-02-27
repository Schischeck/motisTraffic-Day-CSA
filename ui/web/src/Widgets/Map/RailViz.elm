module Widgets.Map.RailViz
    exposing
        ( Msg(SetTimeOffset, SetApiError)
        , Model
        , init
        , view
        , update
        , subscriptions
        , mapId
        , flyTo
        )

import Widgets.Map.Port as Port exposing (..)
import Html exposing (Html, Attribute, div, text, span, input, label, i)
import Html.Attributes exposing (..)
import Html.Events exposing (onClick)
import Time exposing (Time)
import Date exposing (Date)
import Task exposing (..)
import Html exposing (Html)
import Maybe.Extra exposing (isJust, isNothing)
import Util.Core exposing ((=>))
import Util.DateFormat exposing (formatTime, formatDateTimeWithSeconds)
import Localization.Base exposing (..)
import Data.Connection.Types exposing (Station, Position)
import Util.Api as Api exposing (ApiError)
import Widgets.Helpers.ApiErrorUtil exposing (errorText)
import Widgets.LoadingSpinner as LoadingSpinner


-- MODEL


type TrainColors
    = DelayColors
    | ClassColors


type alias Model =
    { mapInfo : MapInfo
    , time : Time
    , systemTime : Time
    , timeOffset : Float
    , mouseX : Int
    , mouseY : Int
    , hoveredTrain : Maybe RVTrain
    , hoveredStation : Maybe String
    , trainColors : TrainColors
    , apiError : Maybe ApiError
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
    , trainColors = ClassColors
    , apiError = Nothing
    }
        ! [ Task.perform SetTime Time.now
          , mapInit mapId
          ]



-- UPDATE


type Msg
    = MapUpdate MapInfo
    | SetTime Time
    | SetTimeOffset Float
    | SetTooltip MapTooltip
    | SetTrainColors TrainColors
    | SetApiError (Maybe ApiError)


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

        SetTooltip tt ->
            { model
                | mouseX = tt.mouseX
                , mouseY = tt.mouseY
                , hoveredTrain = tt.hoveredTrain
                , hoveredStation = tt.hoveredStation
            }
                ! []

        SetTrainColors colors ->
            { model | trainColors = colors }
                ! [ mapUseTrainClassColors (colors == ClassColors) ]

        SetApiError err ->
            { model | apiError = err } ! []


flyTo : Position -> Cmd msg
flyTo pos =
    mapFlyTo
        { mapId = mapId
        , lat = pos.lat
        , lng = pos.lng
        }



-- SUBSCRIPTIONS


subscriptions : Model -> Sub Msg
subscriptions model =
    Sub.batch
        [ mapUpdate MapUpdate
        , mapSetTooltip SetTooltip
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
                        , "leaflet-zoom-animated" => True
                        , "train-hover" => (isJust model.hoveredTrain || isJust model.hoveredStation)
                        ]
                    ]
                    []
                ]
            , railVizTooltip model
            , div [ class "map-bottom-overlay" ]
                [ trainColorPickerView locale model
                , simulationTimeOverlay locale model
                ]
            , errorOverlay locale model
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
                |> Basics.max margin
                |> Basics.min (floor model.mapInfo.pixelBounds.width - ttWidth - margin)

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

        hasDelayInfos =
            train.hasDepartureDelayInfo || train.hasArrivalDelayInfo
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
                , div [ classList [ "time" => True, "no-delay-infos" => not hasDelayInfos ] ]
                    [ span [ class "schedule" ] [ text (formatTime schedDep) ]
                    , delayView train.hasDepartureDelayInfo depDelay
                    ]
                ]
            , div [ class "arrival" ]
                [ span [ class "station" ] [ text train.arrivalStation ]
                , div [ classList [ "time" => True, "no-delay-infos" => not hasDelayInfos ] ]
                    [ span [ class "schedule" ] [ text (formatTime schedArr) ]
                    , delayView train.hasArrivalDelayInfo arrDelay
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
                |> Basics.max margin
                |> Basics.min (floor model.mapInfo.pixelBounds.width - ttWidth - margin)

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
            [ class "sim-time-overlay", id "sim-time-overlay" ]
            [ div [ id "railviz-loading-spinner" ] [ LoadingSpinner.view ]
            , div [ class "permalink" ]
                [ i [ class "icon" ] [ text "link" ] ]
            , div [ class "time" ] [ text (formatDateTimeWithSeconds locale.dateConfig simDate) ]
            ]


delayView : Bool -> Int -> Html Msg
delayView hasDelayInfo minutes =
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
        if hasDelayInfo then
            span [ class delayType ] [ text delayText ]
        else
            text ""


trainColorPickerView : Localization -> Model -> Html Msg
trainColorPickerView locale model =
    div [ class "train-color-picker-overlay" ]
        [ div [] [ text (locale.t.railViz.trainColors ++ ":") ]
        , div []
            [ input
                [ type_ "radio"
                , id "train-color-picker-class"
                , name "train-color-picker"
                , checked (model.trainColors == ClassColors)
                , onClick (SetTrainColors ClassColors)
                ]
                []
            , label [ for "train-color-picker-class" ] [ text locale.t.railViz.classColors ]
            ]
        , div []
            [ input
                [ type_ "radio"
                , id "train-color-picker-delay"
                , name "train-color-picker"
                , checked (model.trainColors == DelayColors)
                , onClick (SetTrainColors DelayColors)
                ]
                []
            , label [ for "train-color-picker-delay" ] [ text locale.t.railViz.delayColors ]
            ]
        ]


errorOverlay : Localization -> Model -> Html Msg
errorOverlay locale model =
    case model.apiError of
        Just err ->
            apiErrorOverlay locale err

        Nothing ->
            text ""


apiErrorOverlay : Localization -> ApiError -> Html Msg
apiErrorOverlay locale err =
    let
        errorMsg =
            "RailViz: " ++ (errorText locale err)
    in
        div [ class "railviz-error-overlay", id "railviz-error-overlay" ]
            [ i [ class "icon" ] [ text "error_outline" ]
            , text errorMsg
            ]
