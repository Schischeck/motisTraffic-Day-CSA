module Data.Intermodal.Request exposing (..)

import Json.Encode as Encode
import Date exposing (Date)
import Util.Core exposing ((=>))
import Util.Date exposing (unixTime)
import Data.Connection.Types exposing (Station, Position)
import Data.Intermodal.Types exposing (..)
import Data.Routing.Types exposing (SearchDirection(..), Interval)
import Data.Routing.Request exposing (encodeInputStation, encodeSearchDirection)
import Data.RailViz.Request exposing (encodePosition)


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
                    , "search_type" => Encode.string "Default"
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
