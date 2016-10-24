module Data.ScheduleInfo.Decode exposing (decodeScheduleInfoResponse)

import Data.ScheduleInfo.Types exposing (..)
import Json.Decode as Decode exposing ((:=))
import Json.Decode.Extra exposing ((|:), withDefault, maybeNull)
import Util.Json exposing (decodeDate)


decodeScheduleInfoResponse : Decode.Decoder ScheduleInfo
decodeScheduleInfoResponse =
    Decode.at [ "content" ] decodeScheduleInfo


decodeScheduleInfo : Decode.Decoder ScheduleInfo
decodeScheduleInfo =
    Decode.succeed ScheduleInfo
        |: ("name" := Decode.string)
        |: ("begin" := decodeDate)
        |: ("end" := decodeDate)
