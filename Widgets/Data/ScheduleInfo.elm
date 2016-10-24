module Widgets.Data.ScheduleInfo exposing (ScheduleInfo, request, decodeScheduleInfoResponse)

import Json.Encode as Encode
import Json.Decode as Decode exposing ((:=))
import Json.Decode.Extra exposing ((|:), withDefault, maybeNull)
import Date exposing (Date, fromTime)
import Widgets.ConnectionUtil exposing ((=>))


type alias ScheduleInfo =
    { name : String
    , begin : Date
    , end : Date
    }


decodeScheduleInfoResponse : Decode.Decoder ScheduleInfo
decodeScheduleInfoResponse =
    Decode.at [ "content" ] decodeScheduleInfo


decodeScheduleInfo : Decode.Decoder ScheduleInfo
decodeScheduleInfo =
    Decode.succeed ScheduleInfo
        |: ("name" := Decode.string)
        |: ("begin" := decodeDate)
        |: ("end" := decodeDate)


decodeDate : Decode.Decoder Date
decodeDate =
    Decode.int `Decode.andThen` (Decode.succeed << Date.fromTime << toFloat << \i -> i * 1000)


request : String
request =
    Encode.object
        [ "destination"
            => Encode.object
                [ "type" => Encode.string "Module"
                , "target" => Encode.string "/lookup/schedule_info"
                ]
        , "content_type" => Encode.string "MotisNoMessage"
        , "content"
            => Encode.object
                []
        ]
        |> Encode.encode 0
