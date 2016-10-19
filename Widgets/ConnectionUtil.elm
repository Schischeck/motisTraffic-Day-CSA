module Widgets.ConnectionUtil exposing (..)

import Date exposing (Date)
import Date.Extra.Duration as Duration exposing (DeltaRecord)
import String
import Set exposing (Set)
import Html exposing (Html, div, ul, li, text, span, i)
import Html.Attributes exposing (..)
import Svg
import Svg.Attributes exposing (xlinkHref)
import Widgets.Data.Connection exposing (..)


type alias Journey =
    { connection : Connection
    , trains : List Train
    }


type alias Train =
    { stops : List Stop
    , transports : List TransportInfo
    }


type EventType
    = Departure
    | Arrival


type alias InterchangeInfo =
    { previousArrival : Maybe EventInfo
    , walk : Bool
    }


twoDigits : Int -> String
twoDigits =
    toString >> String.pad 2 '0'


formatTime : Date -> String
formatTime d =
    (Date.hour d |> twoDigits) ++ ":" ++ (Date.minute d |> twoDigits)


last : List a -> Maybe a
last =
    List.foldl (Just >> always) Nothing


departureEvent : Connection -> Maybe EventInfo
departureEvent connection =
    List.head connection.stops |> Maybe.map .departure


arrivalEvent : Connection -> Maybe EventInfo
arrivalEvent connection =
    last connection.stops |> Maybe.map .arrival


departureTime : Connection -> Maybe Date
departureTime connection =
    departureEvent connection `Maybe.andThen` .schedule_time


arrivalTime : Connection -> Maybe Date
arrivalTime connection =
    arrivalEvent connection `Maybe.andThen` .schedule_time


duration : Connection -> Maybe DeltaRecord
duration connection =
    Maybe.map2 Duration.diff (arrivalTime connection) (departureTime connection)


durationText : DeltaRecord -> String
durationText dr =
    if dr.hour > 0 then
        (toString dr.hour) ++ "h " ++ (toString dr.minute) ++ "min"
    else
        (toString dr.minute) ++ "min"


interchanges : Connection -> Int
interchanges connection =
    Basics.max 0 ((List.length <| List.filter .enter connection.stops) - 1)


transportsForRange : Connection -> Int -> Int -> List TransportInfo
transportsForRange connection from to =
    let
        checkMove : Move -> Maybe TransportInfo
        checkMove move =
            case move of
                Transport transport ->
                    if transport.range.from < to && transport.range.to > from then
                        Just transport
                    else
                        Nothing

                Walk _ ->
                    Nothing
    in
        List.filterMap checkMove connection.transports


groupTrains : Connection -> List Train
groupTrains connection =
    let
        indexedStops : List ( Int, Stop )
        indexedStops =
            List.indexedMap (,) connection.stops

        add_stop : List Train -> Stop -> List Train
        add_stop trains stop =
            case List.head trains of
                Just train ->
                    { train | stops = stop :: train.stops }
                        :: (List.tail trains |> Maybe.withDefault [])

                Nothing ->
                    -- should not happen
                    Debug.log "groupTrains: add_stop empty list" []

        finish_train : List Train -> Int -> Int -> List Train
        finish_train trains start_idx end_idx =
            case List.head trains of
                Just train ->
                    { train | transports = transportsForRange connection start_idx end_idx }
                        :: (List.tail trains |> Maybe.withDefault [])

                Nothing ->
                    -- should not happen
                    Debug.log "groupTrains: finish_train empty list" []

        group : ( Int, Stop ) -> ( List Train, Bool, Int ) -> ( List Train, Bool, Int )
        group ( idx, stop ) ( trains, in_train, end_idx ) =
            let
                ( trains', in_train', end_idx' ) =
                    if stop.enter then
                        ( finish_train (add_stop trains stop) idx end_idx, False, -1 )
                    else
                        ( trains, in_train, end_idx )
            in
                if stop.leave then
                    ( add_stop ((Train [] []) :: trains') stop, True, idx )
                else if in_train' then
                    ( add_stop trains' stop, in_train', end_idx' )
                else
                    ( trains', in_train', end_idx' )

        ( trains, _, _ ) =
            List.foldr group ( [], False, -1 ) indexedStops
    in
        trains


transportCategories : Connection -> Set String
transportCategories connection =
    let
        category : Move -> Maybe String
        category move =
            case move of
                Transport t ->
                    Just t.category_name

                Walk _ ->
                    Nothing
    in
        List.filterMap category connection.transports |> Set.fromList


useLineId : Int -> Bool
useLineId class =
    class == 5 || class == 6


transportName : TransportInfo -> String
transportName transport =
    if useLineId transport.class then
        transport.line_id
    else
        transport.name


trainIcon : Int -> String
trainIcon class =
    case class of
        0 ->
            "train"

        1 ->
            "train"

        2 ->
            "train"

        3 ->
            "train"

        4 ->
            "train"

        5 ->
            "sbahn"

        6 ->
            "ubahn"

        7 ->
            "tram"

        _ ->
            "bus"


trainBox : TransportInfo -> Html msg
trainBox t =
    div [ class <| "train-box train-class-" ++ (toString t.class) ]
        [ Svg.svg
            [ Svg.Attributes.class "train-icon" ]
            [ Svg.use [ xlinkHref <| "icons.svg#" ++ (trainIcon t.class) ] [] ]
        , text <| transportName t
        ]


isDelayed : DeltaRecord -> Bool
isDelayed dr =
    dr.minute > 0 || dr.hour > 0 || dr.day > 0


zeroDelay : DeltaRecord -> Bool
zeroDelay dr =
    dr.minute == 0 && dr.hour == 0 && dr.day == 0


delayText : DeltaRecord -> String
delayText dr =
    let
        str =
            abs >> toString
    in
        if dr.day /= 0 then
            (str dr.day) ++ "d " ++ (str dr.hour) ++ "h " ++ (str dr.minute) ++ "min"
        else if dr.hour /= 0 then
            (str dr.hour) ++ "h " ++ (str dr.minute) ++ "min"
        else
            str dr.minute


delay : EventInfo -> Html msg
delay event =
    let
        diff =
            Maybe.map2 Duration.diff event.time event.schedule_time
    in
        case diff of
            Just d ->
                let
                    delayed =
                        isDelayed d
                in
                    span
                        [ class <|
                            if delayed then
                                "delay pos-delay"
                            else
                                "delay neg-delay"
                        ]
                        [ text <|
                            (if delayed || (zeroDelay d) then
                                "+"
                             else
                                "-"
                            )
                                ++ (delayText d)
                        ]

            Nothing ->
                span [ class "delay" ] []


trainsWithInterchangeInfo : List Train -> List ( Train, InterchangeInfo )
trainsWithInterchangeInfo trains =
    let
        arrival : Train -> Maybe EventInfo
        arrival train =
            Maybe.map .arrival (last train.stops)

        hasWalk : Train -> Train -> Bool
        hasWalk from to =
            let
                arrivalStation =
                    Maybe.map .station (last from.stops)

                departureStation =
                    Maybe.map .station (List.head to.stops)
            in
                arrivalStation /= departureStation

        foldTrains : Train -> List ( Train, InterchangeInfo ) -> List ( Train, InterchangeInfo )
        foldTrains train list =
            case (last list) of
                Just ( lastTrain, _ ) ->
                    list
                        ++ [ ( train
                             , { previousArrival = (arrival lastTrain)
                               , walk = hasWalk lastTrain train
                               }
                             )
                           ]

                Nothing ->
                    [ ( train
                      , { previousArrival = Nothing
                        , walk = False
                        }
                      )
                    ]
    in
        List.foldl foldTrains [] trains


(=>) : a -> b -> ( a, b )
(=>) =
    (,)
