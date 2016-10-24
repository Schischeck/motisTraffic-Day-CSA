module Data.Journey.Types exposing (..)

import Data.Connection.Types exposing (..)
import Util.List exposing (..)


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


toJourney : Connection -> Journey
toJourney connection =
    { connection = connection
    , trains = groupTrains connection
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
