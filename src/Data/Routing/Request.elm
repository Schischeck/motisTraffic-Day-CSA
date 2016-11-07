module Data.Routing.Request exposing (..)

import Json.Encode as Encode
import Date exposing (Date)
import Util.Core exposing ((=>))
import Util.Date exposing (unixTime)
import Data.Connection.Types exposing (Station)


type alias RoutingRequest =
    { from : Station
    , to : Station
    , intervalStart : Int
    , intervalEnd : Int
    , minConnectionCount : Int
    }


initialRequest : Int -> Station -> Station -> Date -> RoutingRequest
initialRequest minConnectionCount from to date =
    let
        selectedTime =
            unixTime date

        startTime =
            selectedTime - 3600

        endTime =
            selectedTime + 3600
    in
        { from = from
        , to = to
        , intervalStart = startTime
        , intervalEnd = endTime
        , minConnectionCount = minConnectionCount
        }


encodeRequest : RoutingRequest -> Encode.Value
encodeRequest request =
    Encode.object
        [ "destination"
            => Encode.object
                [ "type" => Encode.string "Module"
                , "target" => Encode.string "/routing"
                ]
        , "content_type" => Encode.string "RoutingRequest"
        , "content"
            => Encode.object
                [ "start_type" => Encode.string "PretripStart"
                , "start"
                    => Encode.object
                        [ "station" => encodeInputStation request.from
                        , "interval"
                            => Encode.object
                                [ "begin" => Encode.int request.intervalStart
                                , "end" => Encode.int request.intervalEnd
                                ]
                        ]
                , "destination" => encodeInputStation request.to
                , "search_type" => Encode.string "Default"
                , "search_dir" => Encode.string "Forward"
                , "min_connection_count" => Encode.int request.minConnectionCount
                , "via" => Encode.list []
                , "additional_edges" => Encode.list []
                ]
        ]


encodeInputStation : Station -> Encode.Value
encodeInputStation station =
    Encode.object
        [ "name" => Encode.string station.name
        , "id" => Encode.string station.id
        ]
