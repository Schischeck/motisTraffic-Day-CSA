module Data.Intermodal.Decode exposing (decodeIntermodalRoutingResponse)

import Data.Intermodal.Types exposing (..)
import Data.Connection.Decode exposing (decodeConnection)
import Json.Decode as Decode exposing (list)
import Json.Decode.Pipeline exposing (decode, required, optional, hardcoded)


decodeIntermodalRoutingResponse : Decode.Decoder IntermodalRoutingResponse
decodeIntermodalRoutingResponse =
    Decode.at [ "content" ] decodeIntermodalRoutingResponseContent


decodeIntermodalRoutingResponseContent : Decode.Decoder IntermodalRoutingResponse
decodeIntermodalRoutingResponseContent =
    decode IntermodalRoutingResponse
        |> required "connections" (list decodeConnection)
