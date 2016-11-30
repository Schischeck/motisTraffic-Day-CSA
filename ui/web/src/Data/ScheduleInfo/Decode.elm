module Data.ScheduleInfo.Decode exposing (decodeScheduleInfoResponse)

import Data.ScheduleInfo.Types exposing (..)
import Json.Decode as Decode exposing (string)
import Json.Decode.Pipeline exposing (decode, required, optional, hardcoded)
import Util.Json exposing (decodeDate)


decodeScheduleInfoResponse : Decode.Decoder ScheduleInfo
decodeScheduleInfoResponse =
    Decode.at [ "content" ] decodeScheduleInfo


decodeScheduleInfo : Decode.Decoder ScheduleInfo
decodeScheduleInfo =
    decode ScheduleInfo
        |> required "name" string
        |> required "begin" decodeDate
        |> required "end" decodeDate
