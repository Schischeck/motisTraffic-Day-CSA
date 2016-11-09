module Data.Routing.Decode exposing (decodeRoutingResponse)

import Data.Routing.Types exposing (..)
import Data.Connection.Decode exposing (decodeConnection)
import Json.Decode as Decode exposing (list)
import Json.Decode.Pipeline exposing (decode, required, optional, hardcoded)
import Util.Json exposing (decodeDate)


decodeRoutingResponse : Decode.Decoder RoutingResponse
decodeRoutingResponse =
    Decode.at [ "content" ] decodeRoutingResponseContent


decodeRoutingResponseContent : Decode.Decoder RoutingResponse
decodeRoutingResponseContent =
    decode RoutingResponse
        |> required "connections" (list decodeConnection)
        |> required "interval_begin" decodeDate
        |> required "interval_end" decodeDate
