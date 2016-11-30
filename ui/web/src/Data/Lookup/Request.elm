module Data.Lookup.Request exposing (..)

import Json.Encode as Encode exposing (string, int)
import Util.Core exposing ((=>))
import Util.Date exposing (unixTime)
import Data.Connection.Types exposing (TripId)


encodeTripToConnection : TripId -> Encode.Value
encodeTripToConnection tripId =
    Encode.object
        [ "destination"
            => Encode.object
                [ "type" => Encode.string "Module"
                , "target" => Encode.string "/trip_to_connection"
                ]
        , "content_type" => Encode.string "TripId"
        , "content" => encodeTripId tripId
        ]


encodeTripId : TripId -> Encode.Value
encodeTripId tripId =
    Encode.object
        [ "station_id" => string tripId.station_id
        , "train_nr" => int tripId.train_nr
        , "time" => int (unixTime tripId.time)
        , "target_station_id" => string tripId.target_station_id
        , "target_time" => int (unixTime tripId.target_time)
        , "line_id" => string tripId.line_id
        ]
