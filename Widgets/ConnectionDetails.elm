-- module Widgets.Connections exposing (Model, Msg(..), init, update, view)


module Widgets.ConnectionDetails exposing (view)

import Html exposing (Html, div, ul, li, text, span, i)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import Html.Lazy exposing (lazy)
import Svg
import Svg.Attributes exposing (xlinkHref)
import String
import Set exposing (Set)
import Widgets.ViewUtil exposing (onStopAll)
import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Debug
import Widgets.Data.Connection exposing (..)
import Widgets.ConnectionUtil exposing (..)


{--
-- MODEL


type alias Model =
    { trains : List Train
    }


type Msg
    = NoOp


init : List Train -> Model
init trains =
    { trains = trains }


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    model ! []
--}
-- VIEW


view : List Train -> Html msg
view trains =
    div [ class "connection-details" ] <|
        List.map trainDetail (trainsWithInterchangeInfo trains)


stopView : EventType -> Stop -> Html msg
stopView eventType stop =
    let
        event : EventInfo
        event =
            if eventType == Departure then
                stop.departure
            else
                stop.arrival
    in
        div [ class "stop" ]
            [ span [ class "time" ] [ text (Maybe.map formatTime event.schedule_time |> Maybe.withDefault "?") ]
            , delay event
            , span [ class "station" ] [ text stop.station.name ]
            ]


trainTopLine : TrainWithInterchangeInfo -> String
trainTopLine train =
    case train.previousArrival of
        Just pa ->
            let
                prevArrivalTime : Maybe Date
                prevArrivalTime =
                    pa.schedule_time

                departureTime : Maybe Date
                departureTime =
                    (List.head train.train.stops) `Maybe.andThen` (.departure >> .schedule_time)

                d : Maybe DeltaRecord
                d =
                    Maybe.map2 Duration.diff departureTime prevArrivalTime
            in
                "Ankunft Gleis " ++ pa.track ++ ", " ++ (Maybe.map durationText d |> Maybe.withDefault "?") ++ " Umstieg"

        -- TODO: Walk
        Nothing ->
            ""


trainDetail : TrainWithInterchangeInfo -> Html msg
trainDetail train =
    let
        transport =
            List.head train.train.transports

        foldStops : Stop -> List (Html msg) -> List (Html msg)
        foldStops stop result =
            case result of
                [] ->
                    [ stopView Arrival stop ]

                _ ->
                    (stopView Departure stop) :: result

        departureStop =
            List.head train.train.stops
    in
        case transport of
            Just t ->
                div [ class <| "train-detail train-class-" ++ (toString t.class) ] <|
                    [ div [ class "left-border" ] []
                    , div [ class "top-border" ] []
                    , (trainBox t)
                    , div [ class "train-top-line" ]
                        [ span [] [ text (trainTopLine train) ] ]
                    , div [ class "train-dep-track" ]
                        [ text <| "Gleis " ++ (Maybe.map (.departure >> .track) departureStop |> Maybe.withDefault "?") ]
                    ]
                        ++ (List.foldr foldStops [] train.train.stops)

            Nothing ->
                text ""
