module Widgets.ConnectionDetails exposing (State, Config(..), Msg, view, init, update)

import Html exposing (Html, div, ul, li, text, span, i)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import Html.Lazy exposing (lazy)
import String
import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Data.Connection.Types as Connection exposing (..)
import Data.Journey.Types as Journey exposing (..)
import Widgets.Helpers.ConnectionUtil exposing (..)
import Util.Core exposing ((=>))
import Util.DateFormat exposing (..)
import Util.List exposing (..)


-- MODEL


type alias State =
    { journey : Journey
    , expanded : List Bool
    }


type Config msg
    = Config
        { internalMsg : Msg -> msg
        , closeMsg : msg
        }


init : Journey -> State
init journey =
    { journey = journey
    , expanded = List.repeat (List.length journey.trains) False
    }


type Msg
    = ToggleExpand Int


update : Msg -> State -> ( State, Cmd Msg )
update msg model =
    case msg of
        ToggleExpand idx ->
            { model | expanded = (toggle model.expanded idx) } ! []


toggle : List Bool -> Int -> List Bool
toggle list idx =
    case list of
        x :: xs ->
            if idx == 0 then
                (not x) :: xs
            else
                x :: (toggle xs (idx - 1))

        [] ->
            []



-- VIEW


view : Config msg -> State -> Html msg
view (Config { internalMsg, closeMsg }) { journey, expanded } =
    let
        trains =
            trainsWithInterchangeInfo journey.trains

        indices =
            [0..List.length trains - 1]

        trainsView =
            List.map3 (trainDetail internalMsg) trains indices expanded

        walkView maybeWalk =
            case maybeWalk of
                Just walk ->
                    [ walkDetail walk ]

                Nothing ->
                    []

        leadingWalkView =
            walkView journey.leadingWalk

        trailingWalkView =
            walkView journey.trailingWalk

        transportsView =
            leadingWalkView ++ trainsView ++ trailingWalkView
    in
        div [ class "connection-details" ]
            [ connectionInfoView closeMsg journey.connection
            , div [ class "connection-journey", id "connection-journey" ]
                transportsView
            ]


connectionInfoView : msg -> Connection -> Html msg
connectionInfoView closeMsg connection =
    div [ class "connection-info" ]
        [ div [ class "pure-g" ]
            [ div [ class "pure-u-3-24" ]
                [ div [ onClick closeMsg, class "back" ]
                    [ i [ class "icon" ] [ text "arrow_back" ] ]
                ]
            , div [ class "pure-u-5-24 connection-times" ]
                [ div [ class "connection-departure" ]
                    [ text (Maybe.map (formatShortDateTime deDateConfig) (departureTime connection) |> Maybe.withDefault "?")
                    ]
                , div [ class "connection-arrival" ]
                    [ text (Maybe.map (formatShortDateTime deDateConfig) (arrivalTime connection) |> Maybe.withDefault "?")
                    ]
                ]
            , div [ class "pure-u-16-24" ]
                [ div [] [ text (Maybe.map (.station >> .name) (List.head connection.stops) |> Maybe.withDefault "?") ]
                , div [] [ text (Maybe.map (.station >> .name) (last connection.stops) |> Maybe.withDefault "?") ]
                , div [ class "summary" ]
                    [ span [ class "duration" ]
                        [ i [ class "icon" ] [ text "schedule" ]
                        , text (Maybe.map durationText (duration connection) |> Maybe.withDefault "?")
                        ]
                    , span [ class "interchanges" ]
                        [ i [ class "icon" ] [ text "transfer_within_a_station" ]
                        , text <| (toString (interchanges connection)) ++ " Umstiege"
                        ]
                    ]
                ]
            ]
        ]


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


directionView : String -> Html msg
directionView direction =
    div [ class "direction" ]
        [ i [ class "icon" ] [ text "arrow_forward" ]
        , text direction
        ]


trainDetail : (Msg -> msg) -> ( Train, InterchangeInfo ) -> Int -> Bool -> Html msg
trainDetail internalMsg ( train, ic ) idx expanded =
    let
        transport =
            List.head train.transports

        departureStop =
            List.head train.stops

        departureTrack =
            (Maybe.map (.departure >> .track) departureStop |> Maybe.withDefault "")

        arrivalStop =
            last train.stops

        intermediateStops =
            train.stops |> List.drop 1 |> dropEnd 1

        hasIntermediateStops =
            not (List.isEmpty intermediateStops)

        topLine =
            trainTopLine ( train, ic )

        duration =
            Maybe.map2 Duration.diff
                (arrivalStop `Maybe.andThen` (.arrival >> .schedule_time))
                (departureStop `Maybe.andThen` (.departure >> .schedule_time))

        durationStr =
            Maybe.map durationText duration |> Maybe.withDefault "?"

        expandIcon =
            if hasIntermediateStops then
                if expanded then
                    "expand_less"
                else
                    "expand_more"
            else
                ""

        intermediateText =
            if hasIntermediateStops then
                "Fahrt " ++ (toString (List.length intermediateStops)) ++ " Stationen"
            else
                "Fahrt ohne Zwischenhalt"

        intermediateToggleOnClick =
            if hasIntermediateStops then
                [ onClick (internalMsg (ToggleExpand idx)) ]
            else
                []

        direction =
            case Maybe.map .direction transport of
                Just d ->
                    if String.isEmpty d then
                        Nothing
                    else
                        Just d

                Nothing ->
                    Nothing
    in
        case transport of
            Just t ->
                div [ class <| "train-detail train-class-" ++ (toString t.class) ] <|
                    [ div [ class "left-border" ] []
                    , div [ class "top-border" ] []
                    , (trainBox LongName t)
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
                    , div [ class "first-stop" ]
                        [ Maybe.map (stopView Departure) departureStop |> Maybe.withDefault (text "") ]
                    , Maybe.map directionView direction |> Maybe.withDefault (text "")
                    , div
                        ([ classList
                            [ "intermediate-stops-toggle" => True
                            , "clickable" => hasIntermediateStops
                            ]
                         ]
                            ++ intermediateToggleOnClick
                        )
                        [ i [ class "icon" ] [ text expandIcon ]
                        , text (intermediateText ++ " (" ++ durationStr ++ ")")
                        ]
                    , div
                        [ classList
                            [ "intermediate-stops" => True
                            , "expanded" => expanded
                            , "collapsed" => not expanded
                            ]
                        ]
                        (List.map (stopView Departure) intermediateStops)
                    , div [ class "last-stop" ]
                        [ Maybe.map (stopView Arrival) arrivalStop |> Maybe.withDefault (text "") ]
                    ]

            Nothing ->
                text ""


walkDetail : JourneyWalk -> Html msg
walkDetail walk =
    let
        durationStr =
            durationText walk.duration
    in
        div [ class <| "train-detail train-class-walk" ] <|
            [ div [ class "left-border" ] []
            , div [ class "top-border" ] []
            , walkBox
            , div [ class "first-stop" ]
                [ stopView Departure walk.from ]
            , div [ class "intermediate-stops-toggle" ]
                [ text ("Fußweg (" ++ durationStr ++ ")")
                ]
            , div [ class "last-stop" ]
                [ stopView Arrival walk.to ]
            ]
