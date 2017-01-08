module Data.Lookup.Request exposing (..)

import Data.Lookup.Types exposing (..)
import Json.Encode as Encode exposing (string, int)
import Util.Core exposing ((=>))
import Util.Date exposing (unixTime)
import Data.Connection.Types exposing (TripId)
import Date exposing (Date)


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


encodeStationEventsRequest : LookupStationEventsRequest -> Encode.Value
encodeStationEventsRequest req =
    Encode.object
        [ "destination"
            => Encode.object
                [ "type" => Encode.string "Module"
                , "target" => Encode.string "/lookup/station_events"
                ]
        , "content_type" => Encode.string "LookupStationEventsRequest"
        , "content"
            => Encode.object
                [ "station_id" => Encode.string req.stationId
                , "interval"
                    => Encode.object
                        [ "begin" => Encode.int req.intervalStart
                        , "end" => Encode.int req.intervalEnd
                        ]
                , "type" => encodeTableType req.tableType
                ]
        ]


encodeTableType : TableType -> Encode.Value
encodeTableType tt =
    case tt of
        ArrivalsAndDepartures ->
            Encode.string "BOTH"

        OnlyArrivals ->
            Encode.string "ONLY_ARRIVALS"

        OnlyDepartures ->
            Encode.string "ONLY_DEPARTURES"


initialStationEventsRequest : String -> Date -> LookupStationEventsRequest
initialStationEventsRequest stationId date =
    let
        selectedTime =
            unixTime date

        startTime =
            selectedTime - 3600

        endTime =
            selectedTime + 3600
    in
        { stationId = stationId
        , intervalStart = startTime
        , intervalEnd = endTime
        , tableType = ArrivalsAndDepartures
        }
