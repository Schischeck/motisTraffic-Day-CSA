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


trainTopLine : ( Train, InterchangeInfo ) -> String
trainTopLine ( train, ic ) =
    case ic.previousArrival of
        Just pa ->
            let
                prevArrivalTime : Maybe Date
                prevArrivalTime =
                    pa.schedule_time

                departureTime : Maybe Date
                departureTime =
                    (List.head train.stops) `Maybe.andThen` (.departure >> .schedule_time)

                arrivalTime : Maybe Date
                arrivalTime =
                    (List.head train.stops) `Maybe.andThen` (.arrival >> .schedule_time)

                d : Maybe DeltaRecord
                d =
                    if ic.walk then
                        Maybe.map2 Duration.diff arrivalTime prevArrivalTime
                    else
                        Maybe.map2 Duration.diff departureTime prevArrivalTime

                dText =
                    Maybe.map durationText d |> Maybe.withDefault "?"

                icText =
                    if ic.walk then
                        dText ++ " Fußweg"
                    else
                        dText ++ " Umstieg"
            in
                if String.isEmpty pa.track then
                    icText
                else
                    "Ankunft Gleis " ++ pa.track ++ ", " ++ icText

        Nothing ->
            ""


trainDetail : ( Train, InterchangeInfo ) -> Html msg
trainDetail ( train, ic ) =
    let
        transport =
            List.head train.transports

        foldStops : Stop -> List (Html msg) -> List (Html msg)
        foldStops stop result =
            case result of
                [] ->
                    [ stopView Arrival stop ]

                _ ->
                    (stopView Departure stop) :: result

        departureStop =
            List.head train.stops

        departureTrack =
            (Maybe.map (.departure >> .track) departureStop |> Maybe.withDefault "")

        topLine =
            trainTopLine ( train, ic )
    in
        case transport of
            Just t ->
                div [ class <| "train-detail train-class-" ++ (toString t.class) ] <|
                    [ div [ class "left-border" ] []
                    , div [ class "top-border" ] []
                    , (trainBox t)
                    , if String.isEmpty topLine then
                        text ""
                      else
                        div [ class "train-top-line" ]
                            [ span [] [ text topLine ] ]
                    , if String.isEmpty departureTrack then
                        text ""
                      else
                        div [ class "train-dep-track" ]
                            [ text <| "Gleis " ++ departureTrack ]
                    ]
                        ++ (List.foldr foldStops [] train.stops)

            Nothing ->
                text ""
