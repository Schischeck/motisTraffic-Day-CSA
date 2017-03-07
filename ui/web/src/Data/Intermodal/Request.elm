module Data.Intermodal.Request exposing (..)

import Json.Encode as Encode
import Date exposing (Date)
import Util.Core exposing ((=>))
import Util.Date exposing (unixTime)
import Data.Connection.Types exposing (Station, Position)
import Data.Intermodal.Types exposing (..)
import Data.Routing.Types
    exposing
        ( SearchDirection(..)
        , Interval
        , SearchType(..)
        )
import Data.Routing.Request
    exposing
        ( encodeInputStation
        , encodeSearchDirection
        , encodeSearchType
        )
import Data.RailViz.Request exposing (encodePosition)


type IntermodalLocation
    = IntermodalStation Station
    | IntermodalPosition Position


type alias PretripSearchOptions =
    { interval : Interval
    , minConnectionCount : Int
    , extendIntervalEarlier : Bool
    , extendIntervalLater : Bool
    }


initialRequest :
    Int
    -> IntermodalLocation
    -> IntermodalLocation
    -> Date
    -> SearchDirection
    -> IntermodalRoutingRequest
initialRequest minConnectionCount from to date searchDirection =
    let
        selectedTime =
            unixTime date

        interval =
            { begin = selectedTime - 3600
            , end = selectedTime + 3600
            }

        options =
            { interval = interval
            , minConnectionCount = minConnectionCount
            , extendIntervalEarlier = True
            , extendIntervalLater = True
            }

        start =
            toIntermodalStart from options

        destination =
            toIntermodalDestination to
    in
        { start = start
        , startModes = []
        , destination = destination
        , destinationModes = []
        , searchType = DefaultSearchType
        , searchDir = searchDirection
        }


toIntermodalStart : IntermodalLocation -> PretripSearchOptions -> IntermodalStart
toIntermodalStart location options =
    case location of
        IntermodalStation s ->
            PretripStart
                { station = s
                , interval = options.interval
                , minConnectionCount = options.minConnectionCount
                , extendIntervalEarlier = options.extendIntervalEarlier
                , extendIntervalLater = options.extendIntervalLater
                }

        IntermodalPosition p ->
            IntermodalPretripStart
                { position = p
                , interval = options.interval
                , minConnectionCount = options.minConnectionCount
                , extendIntervalEarlier = options.extendIntervalEarlier
                , extendIntervalLater = options.extendIntervalLater
                }


toIntermodalDestination : IntermodalLocation -> IntermodalDestination
toIntermodalDestination location =
    case location of
        IntermodalStation s ->
            InputStation s

        IntermodalPosition p ->
            InputPosition p


startToIntermodalLocation : IntermodalStart -> IntermodalLocation
startToIntermodalLocation start =
    case start of
        PretripStart i ->
            IntermodalStation i.station

        IntermodalPretripStart i ->
            IntermodalPosition i.position


destinationToIntermodalLocation : IntermodalDestination -> IntermodalLocation
destinationToIntermodalLocation dest =
    case dest of
        InputStation s ->
            IntermodalStation s

        InputPosition p ->
            IntermodalPosition p


getInterval : IntermodalRoutingRequest -> Interval
getInterval req =
    case req.start of
        IntermodalPretripStart i ->
            i.interval

        PretripStart i ->
            i.interval


setInterval : IntermodalRoutingRequest -> Interval -> IntermodalRoutingRequest
setInterval req interval =
    let
        newStart =
            case req.start of
                IntermodalPretripStart i ->
                    IntermodalPretripStart { i | interval = interval }

                PretripStart i ->
                    PretripStart { i | interval = interval }
    in
        { req | start = newStart }


setPretripSearchOptions :
    IntermodalRoutingRequest
    -> PretripSearchOptions
    -> IntermodalRoutingRequest
setPretripSearchOptions req options =
    let
        newStart =
            case req.start of
                IntermodalPretripStart i ->
                    IntermodalPretripStart
                        { i
                            | interval = options.interval
                            , minConnectionCount = options.minConnectionCount
                            , extendIntervalEarlier = options.extendIntervalEarlier
                            , extendIntervalLater = options.extendIntervalLater
                        }

                PretripStart i ->
                    PretripStart
                        { i
                            | interval = options.interval
                            , minConnectionCount = options.minConnectionCount
                            , extendIntervalEarlier = options.extendIntervalEarlier
                            , extendIntervalLater = options.extendIntervalLater
                        }
    in
        { req | start = newStart }


encodeRequest : IntermodalRoutingRequest -> Encode.Value
encodeRequest request =
    let
        ( startType, start ) =
            encodeIntermodalStart request.start

        ( destinationType, destination ) =
            encodeIntermodalDestination request.destination
    in
        Encode.object
            [ "destination"
                => Encode.object
                    [ "type" => Encode.string "Module"
                    , "target" => Encode.string "/intermodal"
                    ]
            , "content_type" => Encode.string "IntermodalRoutingRequest"
            , "content"
                => Encode.object
                    [ "start_type" => startType
                    , "start" => start
                    , "start_modes"
                        => Encode.list (List.map encodeMode request.startModes)
                    , "destination_type" => destinationType
                    , "destination" => destination
                    , "destination_modes"
                        => Encode.list (List.map encodeMode request.destinationModes)
                    , "search_type" => encodeSearchType DefaultSearchType
                    , "search_dir" => encodeSearchDirection request.searchDir
                    ]
            ]


encodeIntermodalStart : IntermodalStart -> ( Encode.Value, Encode.Value )
encodeIntermodalStart start =
    case start of
        IntermodalPretripStart info ->
            ( Encode.string "IntermodalPretripStart"
            , Encode.object
                [ "position" => encodePosition info.position
                , "interval" => encodeInterval info.interval
                , "min_connection_count"
                    => Encode.int info.minConnectionCount
                , "extend_interval_earlier"
                    => Encode.bool info.extendIntervalEarlier
                , "extend_interval_later"
                    => Encode.bool info.extendIntervalLater
                ]
            )

        PretripStart info ->
            ( Encode.string "PretripStart"
            , Encode.object
                [ "station" => encodeInputStation info.station
                , "interval" => encodeInterval info.interval
                , "min_connection_count"
                    => Encode.int info.minConnectionCount
                , "extend_interval_earlier"
                    => Encode.bool info.extendIntervalEarlier
                , "extend_interval_later"
                    => Encode.bool info.extendIntervalLater
                ]
            )


encodeIntermodalDestination : IntermodalDestination -> ( Encode.Value, Encode.Value )
encodeIntermodalDestination start =
    case start of
        InputStation station ->
            ( Encode.string "InputStation"
            , encodeInputStation station
            )

        InputPosition pos ->
            ( Encode.string "InputPosition"
            , encodePosition pos
            )


encodeInterval : Interval -> Encode.Value
encodeInterval interval =
    Encode.object
        [ "begin" => Encode.int interval.begin
        , "end" => Encode.int interval.end
        ]


encodeMode : Mode -> Encode.Value
encodeMode mode =
    case mode of
        Foot info ->
            Encode.object
                [ "mode_type" => Encode.string "Foot"
                , "mode"
                    => Encode.object
                        [ "max_duration" => Encode.int info.maxDuration ]
                ]

        Bike info ->
            Encode.object
                [ "mode_type" => Encode.string "Bike"
                , "mode"
                    => Encode.object
                        [ "max_duration" => Encode.int info.maxDuration ]
                ]
