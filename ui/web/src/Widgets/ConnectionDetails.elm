module Widgets.ConnectionDetails
    exposing
        ( State
        , Config(..)
        , Msg
        , view
        , init
        , update
        , getJourney
        )

import Html exposing (Html, div, ul, li, text, span, i)
import Html.Attributes exposing (..)
import Html.Events exposing (onInput, onMouseOver, onFocus, onClick, keyCode, on)
import String
import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import Data.Connection.Types as Connection exposing (..)
import Data.Journey.Types as Journey exposing (..)
import Widgets.Helpers.ConnectionUtil exposing (..)
import Util.Core exposing ((=>))
import Util.DateFormat exposing (..)
import Util.List exposing (..)
import Localization.Base exposing (..)
import Navigation


-- MODEL


type alias State =
    { journey : Journey
    , expanded : List Bool
    }


type Config msg
    = Config
        { internalMsg : Msg -> msg
        , selectTripMsg : Int -> msg
        }


init : Bool -> Journey -> State
init expanded journey =
    { journey = journey
    , expanded = List.repeat (List.length journey.trains) expanded
    }


type Msg
    = ToggleExpand Int
    | GoBack


update : Msg -> State -> ( State, Cmd Msg )
update msg model =
    case msg of
        ToggleExpand idx ->
            { model | expanded = (toggle model.expanded idx) } ! []

        GoBack ->
            model ! [ Navigation.back 1 ]


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


getJourney : State -> Journey
getJourney state =
    state.journey



-- VIEW


view : Config msg -> Localization -> Date -> State -> Html msg
view (Config { internalMsg, selectTripMsg }) locale currentTime { journey, expanded } =
    let
        trains =
            trainsWithInterchangeInfo journey.trains

        indices =
            List.range 0 (List.length trains - 1)

        trainsView =
            List.map3
                (trainDetail journey.isSingleCompleteTrip internalMsg selectTripMsg locale currentTime)
                trains
                indices
                expanded

        walkView maybeWalk =
            case maybeWalk of
                Just walk ->
                    [ walkDetail locale currentTime walk ]

                Nothing ->
                    []

        leadingWalkView =
            walkView journey.leadingWalk

        trailingWalkView =
            walkView journey.trailingWalk

        transportsView =
            leadingWalkView ++ trainsView ++ trailingWalkView
    in
        div
            [ classList
                [ "connection-details" => True
                , "trip-view" => journey.isSingleCompleteTrip
                ]
            ]
            [ connectionInfoView internalMsg locale journey.connection
            , div [ class "connection-journey", id "connection-journey" ]
                transportsView
            ]


connectionInfoView : (Msg -> msg) -> Localization -> Connection -> Html msg
connectionInfoView internalMsg { t, dateConfig } connection =
    let
        depTime =
            departureTime connection |> Maybe.withDefault (Date.fromTime 0)

        arrTime =
            arrivalTime connection |> Maybe.withDefault (Date.fromTime 0)

        depTimeText =
            formatTime depTime

        arrTimeText =
            formatTime arrTime

        dateText =
            formatDate dateConfig depTime
    in
        div [ class "connection-info" ]
            [ div [ class "header" ]
                [ div [ onClick (internalMsg GoBack), class "back" ]
                    [ i [ class "icon" ] [ text "arrow_back" ] ]
                , div [ class "details" ]
                    [ div [ class "date" ] [ text dateText ]
                    , div [ class "connection-times" ]
                        [ div [ class "times" ]
                            [ div [ class "connection-departure" ]
                                [ text depTimeText
                                ]
                            , div [ class "connection-arrival" ]
                                [ text arrTimeText
                                ]
                            ]
                        , div [ class "locations" ]
                            [ div [] [ text (Maybe.map (.station >> .name) (List.head connection.stops) |> Maybe.withDefault "?") ]
                            , div [] [ text (Maybe.map (.station >> .name) (last connection.stops) |> Maybe.withDefault "?") ]
                            ]
                        ]
                    , div [ class "summary" ]
                        [ span [ class "duration" ]
                            [ i [ class "icon" ] [ text "schedule" ]
                            , text (Maybe.map durationText (duration connection) |> Maybe.withDefault "?")
                            ]
                        , span [ class "interchanges" ]
                            [ i [ class "icon" ] [ text "transfer_within_a_station" ]
                            , text <| t.connections.interchanges (interchanges connection)
                            ]
                        ]
                    ]
                , div [ class "actions" ]
                    [ i [ class "icon" ] [ text "save" ]
                    , i [ class "icon" ] [ text "share" ]
                    ]
                ]
            ]


type StopViewType
    = CompactStopView
    | DetailedStopView


stopView : StopViewType -> EventType -> Localization -> Date -> Stop -> Html msg
stopView stopViewType eventType locale currentTime stop =
    let
        event : EventInfo
        event =
            if eventType == Departure then
                stop.departure
            else
                stop.arrival

        timeView e =
            span [ class (timestampClass currentTime e) ]
                [ text (Maybe.map formatTime e.schedule_time |> Maybe.withDefault "?") ]

        timeCell =
            case stopViewType of
                CompactStopView ->
                    [ timeView event ]

                DetailedStopView ->
                    [ div [ class "arrival" ] [ timeView stop.arrival ]
                    , div [ class "departure" ] [ timeView stop.departure ]
                    ]

        delayCell =
            case stopViewType of
                CompactStopView ->
                    [ delay event ]

                DetailedStopView ->
                    [ div [ class "arrival" ] [ delay stop.arrival ]
                    , div [ class "departure" ] [ delay stop.departure ]
                    ]

        trackCell =
            if String.isEmpty event.track then
                text ""
            else
                div [ class "track" ]
                    [ span []
                        [ text (locale.t.connections.track ++ " " ++ event.track) ]
                    ]
    in
        div [ class "stop" ]
            [ div [ class "time" ] timeCell
            , div [ class "delay" ] delayCell
            , div [ class "station" ] [ span [] [ text stop.station.name ] ]
            , trackCell
            ]


timestampClass : Date -> EventInfo -> String
timestampClass currentTime event =
    if eventIsInThePast currentTime event then
        "past-event"
    else
        "future-event"


trainTopLine : Localization -> ( Train, InterchangeInfo ) -> String
trainTopLine locale ( train, ic ) =
    case ic.previousArrival of
        Just pa ->
            let
                prevArrivalTime : Maybe Date
                prevArrivalTime =
                    pa.schedule_time

                departureTime : Maybe Date
                departureTime =
                    (List.head train.stops) |> Maybe.andThen (.departure >> .schedule_time)

                arrivalTime : Maybe Date
                arrivalTime =
                    (List.head train.stops) |> Maybe.andThen (.arrival >> .schedule_time)

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
                        locale.t.connections.walkDuration dText
                    else
                        locale.t.connections.interchangeDuration dText
            in
                if String.isEmpty pa.track then
                    icText
                else
                    (locale.t.connections.arrivalTrack pa.track) ++ ", " ++ icText

        Nothing ->
            ""


directionView : String -> Html msg
directionView direction =
    div [ class "direction" ]
        [ i [ class "icon" ] [ text "arrow_forward" ]
        , text direction
        ]


trainDetail :
    Bool
    -> (Msg -> msg)
    -> (Int -> msg)
    -> Localization
    -> Date
    -> ( Train, InterchangeInfo )
    -> Int
    -> Bool
    -> Html msg
trainDetail isTripView internalMsg selectTripMsg locale currentTime ( train, ic ) idx expanded =
    let
        transport =
            List.head train.transports

        departureStop =
            List.head train.stops

        departureTrack =
            Maybe.map (.departure >> .track) departureStop |> Maybe.withDefault ""

        arrivalStop =
            last train.stops

        arrivalTrack =
            Maybe.map (.arrival >> .track) arrivalStop |> Maybe.withDefault ""

        intermediateStops =
            train.stops |> List.drop 1 |> dropEnd 1

        hasIntermediateStops =
            not (List.isEmpty intermediateStops)

        topLine =
            trainTopLine locale ( train, ic )

        duration =
            Maybe.map2 Duration.diff
                (arrivalStop |> Maybe.andThen (.arrival >> .schedule_time))
                (departureStop |> Maybe.andThen (.departure >> .schedule_time))

        durationStr =
            Maybe.map durationText duration |> Maybe.withDefault "?"

        expandIcon =
            if hasIntermediateStops then
                if expanded then
                    [ i [ class "icon" ] [ text "expand_more" ]
                    , i [ class "icon" ] [ text "expand_less" ]
                    ]
                else
                    [ i [ class "icon" ] [ text "expand_less" ]
                    , i [ class "icon" ] [ text "expand_more" ]
                    ]
            else
                []

        intermediateText =
            locale.t.connections.tripIntermediateStops (List.length intermediateStops)

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

        trainBoxOnClick =
            if isTripView then
                []
            else
                [ onClick (selectTripMsg idx) ]

        intermediateStopViewType =
            if isTripView then
                DetailedStopView
            else
                CompactStopView
    in
        case transport of
            Just t ->
                div
                    [ class <| "train-detail train-class-" ++ (toString t.class) ]
                    [ div [ class "left-border" ] []
                    , div [ class "top-border" ] []
                    , div trainBoxOnClick
                        [ trainBox LongName locale t ]
                    , if String.isEmpty topLine then
                        text ""
                      else
                        div [ class "train-top-line" ]
                            [ span [] [ text topLine ] ]
                    , if String.isEmpty departureTrack then
                        text ""
                      else
                        div [ class "train-dep-track" ]
                            [ text <| locale.t.connections.track ++ " " ++ departureTrack ]
                    , div [ class "first-stop" ]
                        [ Maybe.map
                            (stopView CompactStopView Departure locale currentTime)
                            departureStop
                            |> Maybe.withDefault (text "")
                        ]
                    , Maybe.map directionView direction |> Maybe.withDefault (text "")
                    , div
                        ([ classList
                            [ "intermediate-stops-toggle" => True
                            , "clickable" => hasIntermediateStops
                            ]
                         ]
                            ++ intermediateToggleOnClick
                        )
                        [ div [ class "expand-icon" ] expandIcon
                        , span [] [ text (intermediateText ++ " (" ++ durationStr ++ ")") ]
                        ]
                    , div
                        [ classList
                            [ "intermediate-stops" => True
                            , "expanded" => expanded
                            , "collapsed" => not expanded
                            ]
                        ]
                        (List.map
                            (stopView intermediateStopViewType Departure locale currentTime)
                            intermediateStops
                        )
                    , div [ class "last-stop" ]
                        [ Maybe.map
                            (stopView CompactStopView Arrival locale currentTime)
                            arrivalStop
                            |> Maybe.withDefault (text "")
                        , if String.isEmpty arrivalTrack then
                            text ""
                          else
                            div [ class "train-arr-track" ]
                                [ text <| locale.t.connections.track ++ " " ++ arrivalTrack ]
                        ]
                    ]

            Nothing ->
                text ""


walkDetail : Localization -> Date -> JourneyWalk -> Html msg
walkDetail locale currentTime walk =
    let
        durationStr =
            durationText walk.duration
    in
        div [ class <| "train-detail train-class-walk" ] <|
            [ div [ class "left-border" ] []
            , div [ class "top-border" ] []
            , walkBox
            , div [ class "first-stop" ]
                [ stopView CompactStopView Departure locale currentTime walk.from ]
            , div [ class "intermediate-stops-toggle" ]
                [ div [ class "expand-icon" ] []
                , span [] [ text <| locale.t.connections.tripWalk durationStr ]
                ]
            , div [ class "last-stop" ]
                [ stopView CompactStopView Arrival locale currentTime walk.to ]
            ]
