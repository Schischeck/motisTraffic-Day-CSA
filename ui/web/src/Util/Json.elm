module Util.Json exposing (..)

import Json.Decode as Decode
import Date exposing (Date, fromTime)


decodeDate : Decode.Decoder Date
decodeDate =
    Decode.int |> Decode.andThen (Decode.succeed << Date.fromTime << toFloat << \i -> i * 1000)
