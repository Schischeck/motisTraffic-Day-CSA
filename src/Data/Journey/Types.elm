module Data.Journey.Types
    exposing
        ( Journey
        , Train
        , JourneyWalk
        , EventType(..)
        , InterchangeInfo
        , toJourney
        , trainsWithInterchangeInfo
        )

import Date.Extra.Duration as Duration
import Data.Connection.Types exposing (..)
import Util.List exposing (..)


type alias Journey =
    { connection : Connection
    , trains : List Train
    , leadingWalk : Maybe JourneyWalk
    , trailingWalk : Maybe JourneyWalk
    , isSingleCompleteTrip : Bool
    }


type alias Train =
    { stops : List Stop
    , transports : List TransportInfo
    , trip : Maybe TripId
    }


type alias JourneyWalk =
    { from : Stop
    , to : Stop
    , duration : Duration.DeltaRecord
    }


type EventType
    = Departure
    | Arrival


type alias InterchangeInfo =
    { previousArrival : Maybe EventInfo
    , walk : Bool
    }


toJourney : Connection -> Journey
toJourney connection =
    { connection = connection
    , trains = groupTrains connection
    , leadingWalk = extractLeadingWalk connection
    , trailingWalk = extractTrailingWalk connection
    , isSingleCompleteTrip = False
    }


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
                    let
                        transports =
                            transportsForRange connection start_idx end_idx

                        trip =
                            List.head transports
                                `Maybe.andThen` (tripIdForTransport connection)
                    in
                        { train
                            | transports = transports
                            , trip = trip
                        }
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
                if stop.exit then
                    ( add_stop ((Train [] [] Nothing) :: trains') stop, True, idx )
                else if in_train' then
                    ( add_stop trains' stop, in_train', end_idx' )
                else
                    ( trains', in_train', end_idx' )

        ( trains, _, _ ) =
            List.foldr group ( [], False, -1 ) indexedStops
    in
        trains


extractLeadingWalk : Connection -> Maybe JourneyWalk
extractLeadingWalk connection =
    (getWalkFrom 0 connection) `Maybe.andThen` (toJourneyWalk connection)


extractTrailingWalk : Connection -> Maybe JourneyWalk
extractTrailingWalk connection =
    let
        lastStopIdx =
            (List.length connection.stops) - 1
    in
        (getWalkTo lastStopIdx connection) `Maybe.andThen` (toJourneyWalk connection)



-- TODO: combine adjacent walks


getWalk : (WalkInfo -> Bool) -> Connection -> Maybe WalkInfo
getWalk filter connection =
    let
        checkMove : Move -> Maybe WalkInfo
        checkMove move =
            case move of
                Walk walk ->
                    if filter walk then
                        Just walk
                    else
                        Nothing

                Transport _ ->
                    Nothing
    in
        List.filterMap checkMove connection.transports |> List.head


getWalkTo : Int -> Connection -> Maybe WalkInfo
getWalkTo to =
    getWalk (\w -> w.range.to == to)


getWalkFrom : Int -> Connection -> Maybe WalkInfo
getWalkFrom to =
    getWalk (\w -> w.range.from == to)


toJourneyWalk : Connection -> WalkInfo -> Maybe JourneyWalk
toJourneyWalk connection walkInfo =
    let
        fromStop =
            connection.stops !! walkInfo.range.from

        toStop =
            connection.stops !! walkInfo.range.to

        getWalkDuration from to =
            Maybe.map2
                Duration.diff
                (to.arrival.schedule_time)
                (from.departure.schedule_time)
                |> Maybe.withDefault Duration.zeroDelta

        makeJourneyWalk from to =
            { from = from
            , to = to
            , duration = getWalkDuration from to
            }
    in
        Maybe.map2 makeJourneyWalk fromStop toStop


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
