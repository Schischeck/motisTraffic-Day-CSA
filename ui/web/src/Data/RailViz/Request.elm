module Data.RailViz.Request exposing (..)

import Json.Encode as Encode
import Date exposing (Date)
import Util.Core exposing ((=>))
import Util.Date exposing (unixTime)
import Data.Connection.Types exposing (Position)
import Data.RailViz.Types exposing (RailVizTrainsRequest)


encodeRequest : RailVizTrainsRequest -> Encode.Value
encodeRequest request =
    Encode.object
        [ "destination"
            => Encode.object
                [ "type" => Encode.string "Module"
                , "target" => Encode.string "/railviz/get_trains"
                ]
        , "content_type" => Encode.string "RailVizTrainsRequest"
        , "content"
            => Encode.object
                [ "corner1" => encodePosition request.corner1
                , "corner2" => encodePosition request.corner2
                , "start_time" => encodeDate request.startTime
                , "end_time" => encodeDate request.endTime
                , "max_trains" => Encode.int request.maxTrains
                ]
        ]


encodePosition : Position -> Encode.Value
encodePosition pos =
    Encode.object
        [ "lat" => Encode.float pos.lat
        , "lng" => Encode.float pos.lng
        ]


encodeDate : Date -> Encode.Value
encodeDate date =
    Encode.int (unixTime date)
