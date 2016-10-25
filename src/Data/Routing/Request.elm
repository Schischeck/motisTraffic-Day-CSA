module Data.Routing.Request exposing (..)

import Json.Encode as Encode
import Date exposing (Date)
import Util.Core exposing ((=>))
import Util.Date exposing (unixTime)


type alias RoutingRequest =
    { from : String
    , to : String
    , intervalStart : Int
    , intervalEnd : Int
    }


initialRequest : String -> String -> Date -> RoutingRequest
initialRequest from to date =
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
                        [ "station"
                            => Encode.object
                                [ "name" => Encode.string request.from
                                , "id" => Encode.string ""
                                ]
                        , "interval"
                            => Encode.object
                                [ "begin" => Encode.int request.intervalStart
                                , "end" => Encode.int request.intervalEnd
                                ]
                        ]
                , "destination"
                    => Encode.object
                        [ "name" => Encode.string request.to
                        , "id" => Encode.string ""
                        ]
                , "search_type" => Encode.string "Default"
                , "search_dir" => Encode.string "Forward"
                , "via" => Encode.list []
                , "additional_edges" => Encode.list []
                ]
        ]
