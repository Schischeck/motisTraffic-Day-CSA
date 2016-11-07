module Data.Routing.Decode exposing (decodeRoutingResponse)

import Data.Routing.Types exposing (..)
import Data.Connection.Decode exposing (decodeConnection)
import Json.Decode as Decode exposing ((:=))
import Json.Decode.Extra exposing ((|:), withDefault, maybeNull)
import Util.Json exposing (decodeDate)


decodeRoutingResponse : Decode.Decoder RoutingResponse
decodeRoutingResponse =
    Decode.at [ "content" ] decodeRoutingResponseContent


decodeRoutingResponseContent : Decode.Decoder RoutingResponse
decodeRoutingResponseContent =
    Decode.succeed RoutingResponse
        |: ("connections" := Decode.list decodeConnection)
        |: ("interval_begin" := decodeDate)
        |: ("interval_end" := decodeDate)
